#pragma once
#include <chrono>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

#include "TaskScheduler.h"
#include "Timer/Timer.h"

class SimpleTaskScheduler final : public TaskScheduler {
    using Task = std::function<void()>;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

public:
    explicit SimpleTaskScheduler(std::uint32_t threadPoolSize = std::thread::hardware_concurrency());
    ~SimpleTaskScheduler() override;

    void addStopCallback(Task stopCallback) override;

    void scheduleRepeated(Task task, Duration sleepTime) override;

    void schedule(Task task) override;

    void stop() override;

    void wait() override;

private:
    void workerLoop(std::stop_token stopToken);

    void onStopTimeout();

    Timer m_timer;

    std::vector<std::jthread> m_threadPool;
    std::vector<std::jthread> m_threadsForRepeated;
    std::queue<Task> m_tasks;

    std::mutex m_mutex;
    std::condition_variable m_cv;

    std::atomic_bool m_isRunning{true};
    std::mutex m_waitMtx;
    std::condition_variable m_waitCv;

    std::mutex m_onStopMtx;
    std::vector<Task> m_onStopCallbacks;
    std::atomic_bool m_stopCallbacksExecuted{false};
};
