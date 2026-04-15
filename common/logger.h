#ifndef MINI_XR_LOGGER_H
#define MINI_XR_LOGGER_H

#include <string>

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

void log_message(LogLevel level, const std::string& component,
                 const std::string& message);

class Logger {
public:
    explicit Logger(const std::string& component);

    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

private:
    std::string component_;
};

#endif
