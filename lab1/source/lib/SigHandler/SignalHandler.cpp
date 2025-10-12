#include "SignalHandler.h"

#include <csignal>
#include <syslog.h>

void SignalHandler::handleSignal(int signal) {
    switch (signal) {
        case SIGHUP:
            // TODO reload config из синглтона
        case SIGTERM:
            // TODO graceful shutdown
        default:
            syslog(LOG_WARNING, "Unexpected signal %d", signal);
    }
}
