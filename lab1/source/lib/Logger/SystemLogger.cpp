#include "SystemLogger.h"

#include <sys/syslog.h>

std::string SystemLogger::m_name;

SystemLogger::SystemLogger(const std::string &name) {
    m_name = name;
    openlog(m_name.c_str(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
}

void SystemLogger::log(LogLevel level, const std::string &message) {
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
    syslog(priority, "%s", message.c_str());
}
