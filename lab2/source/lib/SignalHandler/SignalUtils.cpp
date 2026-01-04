#include "SignalUtils.h"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>

#include "ISignalHandler.h"

namespace {

std::atomic<ISignalHandler*> g_registry[NSIG];

void commonTrampoline(int signo, siginfo_t* info, void* ctx) {
    (void)ctx;

    if (signo <= 0 || signo >= NSIG || info == nullptr) {
        return;
    }

    ISignalHandler* handler = g_registry[signo].load(std::memory_order_acquire);
    if (!handler) {
        return;
    }

    handler->onRawSignal(info->si_pid, info->si_value.sival_int);
}

}  // namespace

namespace SignalUtils {

void installHandler(int signo, ISignalHandler* handler) {
    if (signo <= 0 || signo >= NSIG) {
        return;
    }

    g_registry[signo].store(handler, std::memory_order_release);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = &commonTrampoline;

    if (sigaction(signo, &sa, nullptr) == -1) {
        perror("sigaction");
    }
}

void uninstallHandler(int signo, ISignalHandler* handler) {
    if (signo <= 0 || signo >= NSIG) {
        return;
    }

    ISignalHandler* current = g_registry[signo].load(std::memory_order_acquire);

    if (current == handler) {
        g_registry[signo].store(nullptr, std::memory_order_release);
    }
}

void sendUsr1(pid_t pid, ClientConnType connType) {
    sigval v{};
    v.sival_int = static_cast<int>(connType);

    if (sigqueue(pid, SIGUSR1, v) == -1) {
        perror("sigqueue SIGUSR1");
    }
}

void sendUsr2(pid_t pid, std::uint64_t clientId) {
    sigval v{};
    v.sival_int = clientId;

    if (sigqueue(pid, SIGUSR2, v) == -1) {
        perror("sigqueue SIGUSR2");
    }
}
}  // namespace SignalUtils
