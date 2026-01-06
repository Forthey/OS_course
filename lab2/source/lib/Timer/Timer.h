#pragma once

#include <atomic>
#include <chrono>
#include <functional>

class TimerManager;

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::milliseconds;
    using Callback = std::function<void()>;

    Timer(Duration duration, Callback cb);
    ~Timer();

    void start();
    void stop();

    void update(Duration delta);

    bool isRunning() const { return m_running; }

private:
    std::atomic<bool> m_running = false;
    Callback m_callback;

    Duration m_duration;
    Clock::time_point m_currentTime;
    Clock::time_point m_deadline{};
};
