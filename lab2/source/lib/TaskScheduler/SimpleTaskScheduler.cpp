#include "SimpleTaskScheduler.h"

#include "Console.h"

SimpleTaskScheduler::SimpleTaskScheduler(std::uint32_t threadPoolSize)
    : m_timer{std::chrono::seconds{5}, [this] { onStopTimeout(); }}
    , m_threadPool{threadPoolSize} {
    for (auto& thread : m_threadPool) {
        thread = std::jthread([this](std::stop_token stopToken) { workerLoop(stopToken); });
    }
}

SimpleTaskScheduler::~SimpleTaskScheduler() {
    stop();
    m_timer.start();
}

void SimpleTaskScheduler::addStopCallback(Task stopCallback) {
    std::lock_guard lock{m_onStopMtx};
    m_onStopCallbacks.emplace_back(std::move(stopCallback));
}

void SimpleTaskScheduler::scheduleRepeated(Task task, Duration sleepTime) {
    m_threadsForRepeated.emplace_back([task = std::move(task), sleepTime](std::stop_token stopToken) {
        while (!stopToken.stop_requested()) {
            try {
                task();
            } catch (const std::exception& e) {
                consoleSrv().info(std::format("Caught exception in task: {}", e.what()));
            } catch (...) {
                consoleSrv().info("Caught unknown exception in task");
            }
            std::this_thread::sleep_for(sleepTime);
        }
    });
}

void SimpleTaskScheduler::schedule(Task task) {
    {
        std::lock_guard lock{m_mutex};
        m_tasks.push(std::move(task));
    }
    m_cv.notify_one();
}

void SimpleTaskScheduler::stop() {
    for (auto& thread : m_threadPool) {
        thread.request_stop();
    }
    for (auto& thread : m_threadsForRepeated) {
        thread.request_stop();
    }
    m_isRunning = false;
    m_cv.notify_all();

    if (bool notExecuted = false; m_stopCallbacksExecuted.compare_exchange_strong(notExecuted, true)) {
        std::vector<Task> callbacks;
        {
            std::unique_lock lock{m_onStopMtx};
            callbacks = m_onStopCallbacks;
        }

        for (auto& callback : callbacks) {
            try {
                callback();
            } catch (const std::exception& e) {
                consoleSrv().info(std::format("Caught exception in stop callback: {}", e.what()));
            } catch (...) {
                consoleSrv().info("Caught unknown exception in stop callback");
            }
        }
    }

    m_waitCv.notify_all();
}

void SimpleTaskScheduler::wait() {
    std::unique_lock lock{m_waitMtx};
    m_waitCv.wait(lock, [&] { return !m_isRunning; });
}

void SimpleTaskScheduler::workerLoop(std::stop_token stopToken) {
    while (true) {
        Task task;
        {
            std::unique_lock lock{m_mutex};
            m_cv.wait(lock, [&] { return stopToken.stop_requested() || !m_tasks.empty(); });

            if (stopToken.stop_requested()) {
                break;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        try {
            task();
        } catch (const std::exception& e) {
            consoleSrv().info(std::format("Caught exception in task: {}", e.what()));
        } catch (...) {
            consoleSrv().info("Caught unknown exception in task");
        }
    }
}

void SimpleTaskScheduler::onStopTimeout() {
    consoleSrv().info("Graceful stop timeout");
    std::terminate();
}
