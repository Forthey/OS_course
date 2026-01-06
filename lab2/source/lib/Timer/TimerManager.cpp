#include "TimerManager.h"

#include <algorithm>

#include "Timer.h"

TimerManager& TimerManager::instance() {
    static TimerManager timerManager;
    return timerManager;
}

TimerManager::TimerManager() { m_worker = std::jthread{&TimerManager::threadFunc, this}; }

TimerManager::~TimerManager() { shutdown(); }

void TimerManager::shutdown() { m_stop = true; }

void TimerManager::addTimer(Timer* timer) {
    std::unique_lock lock{m_mutex};
    if (std::ranges::find(m_timers, timer) == m_timers.end()) {
        m_timers.push_back(timer);
    }
}

void TimerManager::removeTimer(Timer* timer) {
    std::unique_lock lock{m_mutex};
    if (auto iter = std::ranges::find(m_timers, timer); iter != m_timers.end()) {
        m_timers.erase(iter);
    }
}

void TimerManager::threadFunc() {
    auto timePoint = Clock::now();
    while (!m_stop) {
        auto currentTimePoint = Clock::now();
        const auto timeDelta = std::chrono::duration_cast<Timer::Duration>(currentTimePoint - timePoint);
        timePoint = currentTimePoint;
        {
            std::shared_lock lock{m_mutex};
            for (const auto& timer : m_timers) {
                timer->update(timeDelta);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
}
