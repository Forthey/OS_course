#pragma once

#include <chrono>
#include <condition_variable>
#include <shared_mutex>
#include <thread>
#include <vector>

class Timer;

class TimerManager {
public:
    using Clock = std::chrono::steady_clock;

    static TimerManager& instance();

    void addTimer(Timer* timer);
    void removeTimer(Timer* timer);

    void shutdown();

private:
    TimerManager();
    ~TimerManager();

    void threadFunc();

    std::shared_mutex m_mutex;
    std::vector<Timer*> m_timers;

    std::atomic<bool> m_stop = false;
    std::jthread m_worker;
};
