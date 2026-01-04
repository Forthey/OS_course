#pragma once

#include <atomic>
#include <cstddef>

#include "ISignalHandler.h"

class MultiSignalHandler : public ISignalHandler {
public:
    struct Event {
        pid_t pid;
        int value;
    };

    MultiSignalHandler(int signo, Callback callback, std::size_t queueSize = 1024);
    ~MultiSignalHandler() override;

    void poll() override;
    void onRawSignal(pid_t pid, int value) override;

private:
    int m_signo;
    Callback m_callback;

    const std::size_t m_capacity;
    Event* m_queue;

    std::atomic<std::size_t> m_head{0};  // читает poll()
    std::atomic<std::size_t> m_tail{0};  // пишет handler
};
