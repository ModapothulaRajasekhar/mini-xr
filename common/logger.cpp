#include "logger.h"
#include <iostream>
#include <ctime>

static std::string now() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

void log_message(LogLevel level,
                 const std::string& component,
                 const std::string& message) {

    const char* level_str =
        (level == LogLevel::INFO)  ? "INFO" :
        (level == LogLevel::WARN)  ? "WARN" :
                                     "ERROR";

    std::cout << "[" << now() << "] "
              << "[" << level_str << "] "
              << "[" << component << "] "
              << message << std::endl;
}

Logger::Logger(const std::string& component)
    : component_(component) {}

void Logger::info(const std::string& msg) {
    log_message(LogLevel::INFO, component_, msg);
}

void Logger::warn(const std::string& msg) {
    log_message(LogLevel::WARN, component_, msg);
}

void Logger::error(const std::string& msg) {
    log_message(LogLevel::ERROR, component_, msg);
}
