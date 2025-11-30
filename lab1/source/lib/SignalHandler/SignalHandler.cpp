#include "SignalHandler.h"

#include <csignal>
#include <cstring>
#include <format>
#include <syslog.h>

#include "Logger/SystemLogger.h"
#include "Observer/Messages.h"

/// Костыль, так как sa_handler принимает только Си-функцию
void handleSignal(const int signal) {
    SignalHandler::instance().handleSignal(signal);
}

SignalHandler::SignalHandler() {
    // Игнорируем SIGCHLD, чтобы не оставлять зомби (если мы форкаем дочерние процессы)
    struct sigaction sa_chld{};
    std::memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = SIG_IGN;
    sa_chld.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa_chld, nullptr);

    for (int sig = 1; sig < NSIG; ++sig) {
        if (sig == SIGKILL || sig == SIGSTOP || sig == SIGCHLD) {
            continue;
        }
        struct sigaction sa{};
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = ::handleSignal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(sig, &sa, nullptr) < 0) {
            SystemLogger::instance().warn(std::format("sigaction({}) failed: {}", strsignal(sig), strerror(errno)));
        }
    }
}

void SignalHandler::handleSignal(const int signal) {
    SystemLogger::instance().info(std::format("Received signal {}", strsignal(signal)));
    switch (signal) {
        case SIGHUP:
            notify(std::make_shared<ReloadConfigRequest>());
            break;
        case SIGTERM:
            notify(std::make_shared<StopRequest>());
            break;
        default:
            SystemLogger::instance().warn(std::format("Unexpected signal {}, ignoring", strsignal(signal)));
    }
}
