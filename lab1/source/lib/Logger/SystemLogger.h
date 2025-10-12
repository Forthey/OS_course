#pragma once
#include "Logger.h"
#include "OnceInstantiated/OnceInstantiated.h"

class SystemLogger : public Logger, public OnceInstantiated<SystemLogger> {
    friend class OnceInstantiated;
public:
    void log(LogLevel level, const std::string &message) override;

protected:
    explicit SystemLogger(const std::string& name);
private:
    static std::string m_name;
};
