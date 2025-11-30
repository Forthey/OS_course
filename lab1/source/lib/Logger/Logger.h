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

    virtual void log(LogLevel level, const std::string &message, int scope) = 0;

    virtual void info(const std::string &message, int scope) { log(INFO, message, scope); }

    virtual void warn(const std::string &message, int scope) { log(WARNING, message, scope); }

    virtual void error(const std::string &message, int scope) { log(ERROR, message, scope); }
};
