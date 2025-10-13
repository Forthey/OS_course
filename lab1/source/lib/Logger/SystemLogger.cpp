#include "SystemLogger.h"

#include <sys/syslog.h>

SystemLogger::SystemLogger(std::string name) : m_name{std::move(name)} {
    openlog(m_name.c_str(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
}

void SystemLogger::log(const LogLevel level, const std::string &message, const int scope) {
    int priority;
    switch (level) {
        case INFO:
            priority = LOG_INFO;
            break;
        case WARNING:
            priority = LOG_WARNING;
            break;
        case ERROR:
            priority = LOG_ERR;
            break;
        default:
            priority = LOG_DEBUG;
            break;
    }

    int priorityScoped;
    switch (scope) {
        case SystemLogger::SYSTEM:
            priorityScoped = priority | LOG_DAEMON;
            break;
        case SystemLogger::LOCAL0:
            priorityScoped = priority | LOG_LOCAL0;
            break;
        default:
            priorityScoped = priority | LOG_DAEMON;
            break;
    }
    syslog(priorityScoped, "%s", message.c_str());
}

SystemLogger::~SystemLogger() {
    closelog();
}
