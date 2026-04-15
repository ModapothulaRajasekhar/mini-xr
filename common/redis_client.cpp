#include "redis_client.h"
#include "logger.h"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>

RedisClient::RedisClient(const std::string& host, int port)
    : host_(host), port_(port) {}

bool RedisClient::connect() {
    log_message(LogLevel::INFO,
                "RedisClient",
                "Connecting to Redis at " + host_ + ":" +
                std::to_string(port_));
    return true;
}

bool RedisClient::set(const std::string& key,
                      const std::string& value) {

    std::string cmd =
        "redis-cli -h " + host_ +
        " -p " + std::to_string(port_) +
        " SET " + key + " \"" + value + "\"";

    log_message(LogLevel::INFO,
                "RedisClient",
                "Executing: " + cmd);

    int ret = system(cmd.c_str());

    return (ret == 0);
}

bool RedisClient::del(const std::string& key) {

    std::string cmd =
        "redis-cli -h " + host_ +
        " -p " + std::to_string(port_) +
        " DEL " + key;

    log_message(LogLevel::INFO,
                "RedisClient",
                "Executing: " + cmd);

    int ret = system(cmd.c_str());

    return (ret == 0);
}

std::string RedisClient::get(const std::string& key) {

    std::string cmd =
        "redis-cli -h " + host_ +
        " -p " + std::to_string(port_) +
        " GET " + key;

    log_message(LogLevel::INFO,
                "RedisClient",
                "Executing: " + cmd);

    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return "";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);

    return result;
}
