#include "logger.h"
#include "redis_client.h"
#include "pubsub.h"

#include <thread>
#include <chrono>

int main() {
    Logger log("protod");
    log.info("Starting Protocol Manager");

    RedisClient redis("127.0.0.1", 6379);
    redis.connect();

    PubSub pubsub("127.0.0.1", 6379);
    pubsub.connect();

    // Simulated protocol route
    std::string route_key = "proto:route:10.1.0.0/24";
    redis.set(
        route_key,
        R"({ "nexthop": "10.0.0.2", "protocol": "ospf", "metric": 10 })"
    );

    // Publish route event
    pubsub.publish("proto.route", route_key);

    log.info("Published protocol route event");

    while (true) {
        log.info("Protocol heartbeat");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
