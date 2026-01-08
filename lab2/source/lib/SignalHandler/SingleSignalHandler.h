#pragma once

#include <atomic>

#include "SignalHandler.h"

class SingleSignalHandler final : public SignalHandler {
public:
    explicit SingleSignalHandler(int signo, Callback callback);
    ~SingleSignalHandler() override;

    void poll() override;
    void onRawSignal(pid_t pid, int data) override;

private:
    int m_signo;
    Callback m_callback;

    std::atomic<bool> m_hasEvent{false};
    std::atomic<pid_t> m_lastPid{0};
    std::atomic<int> m_lastValue{0};
};
