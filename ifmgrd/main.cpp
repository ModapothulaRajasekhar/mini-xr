#include <unistd.h>
#include "logger.h"
#include "redis_client.h"

int main() {
    const std::string component = "ifmgrd";

    log_message(LogLevel::INFO, component, "Starting Interface Manager");

    RedisClient redis("127.0.0.1", 6379);
    if (!redis.connect()) {
        log_message(LogLevel::ERROR, component, "Failed to connect to Redis");
        return 1;
    }

    // Publish dummy interface state
    redis.set("ifmgrd:interfaces:eth0",
              "{ \"admin_state\": \"up\", \"oper_state\": \"up\" }");

    while (true) {
        log_message(LogLevel::INFO, component, "Heartbeat");
        sleep(5);
    }

    return 0;
}
