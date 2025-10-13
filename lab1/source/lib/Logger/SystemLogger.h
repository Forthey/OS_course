#pragma once
#include "Logger.h"
#include "OnceInstantiated/OnceInstantiated.h"

class SystemLogger : public Logger, public OnceInstantiated<SystemLogger> {
    friend class OnceInstantiated;
public:
    enum Facility {
        SYSTEM,
        LOCAL0,
    };

    ~SystemLogger() override;

    void log(LogLevel level, const std::string &message, int scope) override;

    void log(LogLevel level, const std::string &message) { log(level, message, SYSTEM); }

    using Logger::info;
    void info(const std::string &message) { log(INFO, message); }

    using Logger::warn;
    void warn(const std::string &message) { log(WARNING, message); }

    using Logger::error;
    void error(const std::string &message) { log(ERROR, message); }
protected:
    explicit SystemLogger(std::string name);

    const std::string m_name;
};
