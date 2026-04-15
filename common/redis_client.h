#ifndef MINI_XR_REDIS_CLIENT_H
#define MINI_XR_REDIS_CLIENT_H

#include <string>

class RedisClient {
public:
    RedisClient(const std::string& host, int port);

    bool connect();
    bool set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);

private:
    std::string host_;
    int port_;
};

#endif
