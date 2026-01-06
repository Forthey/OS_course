#pragma once

#include <atomic>
#include <cstddef>

#include "Types/SignalHandler.h"

class MultiSignalHandler : public SignalHandler {
public:
    struct Event {
        pid_t pid;
        int value;
    };

    MultiSignalHandler(int signal, Callback callback, std::size_t queueSize = 1024);
    ~MultiSignalHandler() override;

    void poll() override;
    void onRawSignal(pid_t pid, int data) override;

private:
    int m_signal;
    Callback m_callback;

    const std::size_t m_capacity;
    Event* m_queue;

    std::atomic<std::size_t> m_head{0};
    std::atomic<std::size_t> m_tail{0};
};
