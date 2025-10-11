#include "SigHandler.h"

#include <csignal>
#include <syslog.h>

void SigHandler::handleSignal(int signal) {
    switch (signal) {
        case SIGHUP:
            // TODO reload config из синглтона
        case SIGTERM:
            // TODO graceful shutdown
        default:
            syslog(LOG_WARNING, "Unexpected signal %d", signal);
    }
}
