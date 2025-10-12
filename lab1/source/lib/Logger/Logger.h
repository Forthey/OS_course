#pragma once
#include <string>

class Logger {
public:
    virtual ~Logger() = default;

    enum LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    virtual void log(LogLevel level, const std::string& message) = 0;

    virtual void info(const std::string& message) { log(INFO, message); }

    virtual void warn(const std::string& message) { log(WARNING, message); }

    virtual void error(const std::string& message) { log(ERROR, message); }
};
