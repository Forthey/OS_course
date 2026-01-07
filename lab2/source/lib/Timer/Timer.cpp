#include "Timer.h"

#include "TimerManager.h"

Timer::Timer(const Duration duration, Callback cb) : m_callback{std::move(cb)}, m_duration{duration} {
    TimerManager::instance().addTimer(this);
}

Timer::~Timer() {
    stop();
    TimerManager::instance().removeTimer(this);
}

void Timer::start() {
    std::unique_lock lock{m_mutex};
    m_currentTime = Clock::now();
    m_deadline = m_currentTime + m_duration;
    m_running = true;
}

void Timer::stop() { m_running = false; }

void Timer::reset() {
    stop();
    start();
}

void Timer::update(const Duration delta) {
    if (!m_running) {
        return;
    }
    std::unique_lock lock{m_mutex};
    m_currentTime = m_currentTime + delta;
    if (m_currentTime >= m_deadline) {
        m_callback();
        m_running = false;
    }
}
