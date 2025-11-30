#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        {
            std::lock_guard lock{m_mutex};
            m_queue.push(std::move(value));
        }
        m_cv.notify_one();
    }

    T pop() {
        std::unique_lock lock{m_mutex};
        m_cv.wait(lock, [this] { return !m_queue.empty(); });
        T value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    bool try_pop(T &value) {
        std::lock_guard lock{m_mutex};
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};
