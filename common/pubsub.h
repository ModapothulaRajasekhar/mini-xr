#ifndef MINI_XR_PUBSUB_H
#define MINI_XR_PUBSUB_H

#include <string>
#include <functional>

class PubSub {
public:
    using Callback = std::function<void(const std::string&)>;

    PubSub(const std::string& host, int port);

    bool connect();

    // Publisher
    bool publish(const std::string& channel,
                 const std::string& message);

    // Subscriber (blocking loop)
    void subscribe(const std::string& channel,
                   Callback cb);

private:
    std::string host_;
    int port_;
};

#endif
