#include "lsdb.h"
#include "spf.h"
#include "neighbor.h"

#include "../common/logger.h"
#include "../common/redis_client.h"
#include "../common/pubsub.h"

#include <thread>
#include <chrono>
#include <map>
#include <memory>

int main() {

    Logger log("ospfd");
    log.info("Starting OSPF Daemon (True ECMP Topology)");

    const std::string LOCAL_RID = "1.1.1.1";

    LSDB lsdb;
    SPFEngine spf(LOCAL_RID);

    RedisClient redis("127.0.0.1", 6379);
    redis.connect();

    PubSub pubsub("127.0.0.1", 6379);
    pubsub.connect();

    std::map<std::string, std::unique_ptr<Neighbor>> neighbors;
    std::map<std::string, int> local_links;

    // Install 4.4.4.4 LSA (multi-hop node)
    auto install_remote_topology = [&]() {

        RouterLSA lsa4;
        lsa4.router_id = "4.4.4.4";
        lsa4.sequence_number = 1;
        lsa4.age = 0;

        lsa4.links.push_back({"2.2.2.2", 5});
        lsa4.links.push_back({"3.3.3.3", 5});

        lsdb.install(lsa4);
    };

    auto rebuild_local_lsa = [&]() {

        RouterLSA local;
        local.router_id = LOCAL_RID;
        local.sequence_number++;
        local.age = 0;

        for (const auto& [nbr, cost] : local_links)
            local.links.push_back({nbr, cost});

        lsdb.install(local);
    };

    auto add_neighbor = [&](const std::string& rid, int cost) {

        neighbors[rid] = std::make_unique<Neighbor>(
            rid,

            // FULL
            [&, rid, cost]() {

                log.info("Neighbor " + rid + " FULL — installing LSA");

                local_links[rid] = cost;
                rebuild_local_lsa();

                RouterLSA remote;
                remote.router_id = rid;
                remote.sequence_number = 1;
                remote.age = 0;

                remote.links.push_back({LOCAL_RID, cost});
                remote.links.push_back({"4.4.4.4", 5});

                lsdb.install(remote);
                install_remote_topology();
            },

            // DOWN
            [&, rid]() {

                log.warn("Adjacency DOWN — withdrawing routes for " + rid);

                local_links.erase(rid);
                rebuild_local_lsa();
                lsdb.remove(rid);

                pubsub.publish("proto.route.del", "ospf:" + rid);
            }
        );
    };

    // Equal-cost neighbors
    add_neighbor("2.2.2.2", 10);
    add_neighbor("3.3.3.3", 10);

    std::map<std::string, int> previous_cost;

    while (true) {

        for (auto& [rid, nbr] : neighbors) {
            nbr->receive_hello();
            nbr->check_dead_timer();
        }

        if (lsdb.is_dirty()) {

            log.info("LSDB changed — running SPF");

            spf.run(lsdb);

            const auto& distances = spf.get_distances();
            const auto& next_hops = spf.get_next_hops();

            for (const auto& [rid, cost] : distances) {

                if (rid == LOCAL_RID)
                    continue;

                log.info("Publishing ECMP route to " + rid);

                for (const auto& nh : next_hops.at(rid)) {
                    log.info("  → via " + nh +
                             " cost=" + std::to_string(cost));
                }

                pubsub.publish(
                    "proto.route.add",
                    "ospf:" + rid + ":" +
                    std::to_string(cost)
                );
            }

            lsdb.clear_dirty();
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}
