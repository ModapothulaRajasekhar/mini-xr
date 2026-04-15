#include "logger.h"
#include "redis_client.h"
#include "pubsub.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>
#include <sstream>

/*
 * Route structure
 */
struct Route {
    std::string prefix;
    std::string protocol;
    std::string nexthop;
    int metric;
    int admin_distance;
};

/*
 * Candidate RIB
 */
std::map<std::string, std::vector<Route>> candidate_rib;

/*
 * Convergence control
 */
std::map<std::string, std::time_t> last_event_ts;
std::mutex conv_mtx;

constexpr int CONVERGENCE_MS = 200;

/*
 * Admin distance
 */
static int admin_distance(const std::string& proto) {
    if (proto == "static") return 1;
    if (proto == "ospf")   return 110;
    if (proto == "bgp")    return 200;
    return 255;
}

/*
 * ECMP Best route selection
 */
static std::vector<Route>
select_best_ecmp(const std::vector<Route>& routes) {

    if (routes.empty())
        return {};

    int best_admin = routes.front().admin_distance;
    int best_metric = routes.front().metric;

    // Find best admin distance
    for (const auto& r : routes) {
        if (r.admin_distance < best_admin)
            best_admin = r.admin_distance;
    }

    // Among best admin, find best metric
    for (const auto& r : routes) {
        if (r.admin_distance == best_admin &&
            r.metric < best_metric)
            best_metric = r.metric;
    }

    // Collect all equal best routes (ECMP)
    std::vector<Route> best_routes;

    for (const auto& r : routes) {
        if (r.admin_distance == best_admin &&
            r.metric == best_metric) {
            best_routes.push_back(r);
        }
    }

    return best_routes;
}

/*
 * Program ECMP best route
 */
static void program_best_route(const std::string& prefix,
                               const std::vector<Route>& best_routes,
                               RedisClient& redis,
                               Logger& log) {

    if (best_routes.empty())
        return;

    std::string json =
        "{ \"protocol\": \"" + best_routes[0].protocol +
        "\", \"metric\": " +
        std::to_string(best_routes[0].metric) +
        ", \"next_hops\": [";

    for (size_t i = 0; i < best_routes.size(); ++i) {
        json += "\"" + best_routes[i].nexthop + "\"";
        if (i != best_routes.size() - 1)
            json += ", ";
    }

    json += "] }";

    redis.set("rib:best:" + prefix, json);

    log.info("ECMP route installed for " + prefix +
             " with " +
             std::to_string(best_routes.size()) +
             " next-hop(s)");
}

/*
 * Withdraw best route
 */
static void withdraw_best_route(const std::string& prefix,
                                RedisClient& redis,
                                Logger& log) {
    redis.del("rib:best:" + prefix);
    log.info("Best route withdrawn for prefix " + prefix);
}

/*
 * Schedule recompute
 */
static void schedule_recompute(const std::string& prefix,
                               RedisClient& redis,
                               Logger& log) {

    std::time_t scheduled_at = std::time(nullptr);

    {
        std::lock_guard<std::mutex> g(conv_mtx);
        last_event_ts[prefix] = scheduled_at;
    }

    std::thread([prefix, scheduled_at, &redis, &log]() {

        std::this_thread::sleep_for(
            std::chrono::milliseconds(CONVERGENCE_MS)
        );

        {
            std::lock_guard<std::mutex> g(conv_mtx);
            if (last_event_ts[prefix] != scheduled_at)
                return;
        }

        if (candidate_rib.count(prefix) == 0) {
            withdraw_best_route(prefix, redis, log);
            return;
        }

        auto& routes = candidate_rib[prefix];

        auto best_routes = select_best_ecmp(routes);
        program_best_route(prefix, best_routes, redis, log);

    }).detach();
}

/*
 * Parse ospf message:
 * ADD format -> ospf:RID:metric
 * DEL format -> ospf:RID
 */
static bool parse_add(const std::string& msg,
                      std::string& protocol,
                      std::string& rid,
                      int& metric) {

    if (msg.find("ospf:") != 0)
        return false;

    std::stringstream ss(msg);
    std::string token;

    std::vector<std::string> parts;
    while (std::getline(ss, token, ':'))
        parts.push_back(token);

    if (parts.size() != 3)
        return false;

    protocol = parts[0];
    rid      = parts[1];
    metric   = std::stoi(parts[2]);

    return true;
}

static bool parse_del(const std::string& msg,
                      std::string& protocol,
                      std::string& rid) {

    if (msg.find("ospf:") != 0)
        return false;

    std::stringstream ss(msg);
    std::string token;

    std::vector<std::string> parts;
    while (std::getline(ss, token, ':'))
        parts.push_back(token);

    if (parts.size() != 2)
        return false;

    protocol = parts[0];
    rid      = parts[1];

    return true;
}

int main() {

    Logger log("ribd");
    log.info("Starting Routing Information Base Daemon (ECMP Enabled)");

    RedisClient redis("127.0.0.1", 6379);
    redis.connect();

    PubSub pubsub("127.0.0.1", 6379);
    pubsub.connect();

    /*
     * ADD handler
     */
    pubsub.subscribe("proto.route.add",
        [&](const std::string& msg) {

            std::string protocol, rid;
            int metric;

            if (!parse_add(msg, protocol, rid, metric)) {
                log.warn("Ignoring invalid ADD message: " + msg);
                return;
            }

            std::string prefix = rid + "/32";

            log.info("Route ADD received: " + protocol +
                     " " + prefix);

            Route r {
                prefix,
                protocol,
                rid,
                metric,
                admin_distance(protocol)
            };

            candidate_rib[prefix].push_back(r);
            schedule_recompute(prefix, redis, log);
        }
    );

    /*
     * DEL handler
     */
    pubsub.subscribe("proto.route.del",
        [&](const std::string& msg) {

            std::string protocol, rid;

            if (!parse_del(msg, protocol, rid)) {
                log.warn("Ignoring invalid DEL message: " + msg);
                return;
            }

            std::string prefix = rid + "/32";

            log.info("Route DEL received: " +
                     protocol + " " + prefix);

            if (candidate_rib.count(prefix) == 0)
                return;

            auto& routes = candidate_rib[prefix];

            routes.erase(
                std::remove_if(routes.begin(), routes.end(),
                    [&](const Route& r) {
                        return r.protocol == protocol &&
                               r.nexthop == rid;
                    }),
                routes.end()
            );

            if (routes.empty())
                candidate_rib.erase(prefix);

            schedule_recompute(prefix, redis, log);
        }
    );

    while (true) {
        log.info("RIB heartbeat");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
