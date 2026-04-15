#include "pubsub.h"
#include "logger.h"

#include <cstdio>
#include <thread>
#include <string>

PubSub::PubSub(const std::string& host, int port)
    : host_(host), port_(port) {}

bool PubSub::connect() {
    log_message(LogLevel::INFO, "PubSub",
                "Connecting to Redis Pub/Sub at " +
                host_ + ":" + std::to_string(port_));
    return true;
}

bool PubSub::publish(const std::string& channel,
                     const std::string& message) {

    std::string cmd =
        "redis-cli -h " + host_ +
        " -p " + std::to_string(port_) +
        " PUBLISH " + channel +
        " \"" + message + "\"";

    log_message(LogLevel::INFO, "PubSub",
                "Executing: " + cmd);

    std::system(cmd.c_str());
    return true;
}

void PubSub::subscribe(const std::string& channel,
                       Callback cb) {

    log_message(LogLevel::INFO, "PubSub",
                "SUBSCRIBE [" + channel + "]");

    std::string cmd =
        "redis-cli -h " + host_ +
        " -p " + std::to_string(port_) +
        " --raw SUBSCRIBE " + channel;

    std::thread([cmd, cb]() {

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            log_message(LogLevel::ERROR, "PubSub",
                        "Failed to open redis-cli pipe");
            return;
        }

        char buffer[512];

        /*
         Redis --raw output format:

         subscribe
         proto.route.add
         1

         message
         proto.route.add
         ospf:2.2.2.2:10
        */

        enum class State {
            WAITING,
            GOT_MESSAGE,
            GOT_CHANNEL
        };

        State state = State::WAITING;

        while (fgets(buffer, sizeof(buffer), pipe)) {

            std::string line(buffer);

            // Remove newline
            if (!line.empty() && line.back() == '\n')
                line.pop_back();

            if (line == "message") {
                state = State::GOT_MESSAGE;
                continue;
            }

            if (state == State::GOT_MESSAGE) {
                // This is channel name (ignore)
                state = State::GOT_CHANNEL;
                continue;
            }

            if (state == State::GOT_CHANNEL) {
                // This is actual payload
                cb(line);
                state = State::WAITING;
                continue;
            }

            // Ignore:
            // subscribe
            // channel name
            // subscriber count
        }

        pclose(pipe);

    }).detach();
}
