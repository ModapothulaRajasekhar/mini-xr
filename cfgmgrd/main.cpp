#include <unistd.h>
#include <string>
#include "logger.h"
#include "redis_client.h"

static void fake_commit(RedisClient& redis) {
    // Fake candidate configuration
    std::string candidate_config =
        "{ \"interfaces\": { \"eth0\": { \"admin_state\": \"up\" } } }";

    log_message(LogLevel::INFO, "cfgmgrd",
                "Writing candidate configuration");

    redis.set("config:candidate", candidate_config);

    // Fake validation step
    log_message(LogLevel::INFO, "cfgmgrd",
                "Validating candidate configuration");

    sleep(1); // simulate validation delay

    // Commit
    log_message(LogLevel::INFO, "cfgmgrd",
                "Committing configuration");

    redis.set("config:running", candidate_config);
    redis.set("config:commit-id", "commit-001");
}

int main() {
    const std::string component = "cfgmgrd";

    log_message(LogLevel::INFO, component,
                "Starting Configuration Manager");

    RedisClient redis("127.0.0.1", 6379);
    if (!redis.connect()) {
        log_message(LogLevel::ERROR, component,
                    "Failed to connect to Redis");
        return 1;
    }

    // Simulate a commit on startup
    fake_commit(redis);

    while (true) {
        log_message(LogLevel::INFO, component,
                    "Heartbeat");
        sleep(5);
    }

    return 0;
}
