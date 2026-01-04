#pragma once

#include <atomic>

#include "ISignalHandler.h"

class SingleSignalHandler : public ISignalHandler {
public:
    explicit SingleSignalHandler(int signo, Callback callback);
    ~SingleSignalHandler() override;

    void poll() override;
    void onRawSignal(pid_t pid, int value) override;

private:
    int m_signo;
    Callback m_callback;

    std::atomic<bool> m_hasEvent{false};
    std::atomic<pid_t> m_lastPid{0};
    std::atomic<int> m_lastValue{0};
};
