#pragma once
#include <memory>
#include <mutex>

template<typename SingletonClass>
class OnceInstantiated {
protected:
    static std::shared_ptr<SingletonClass> m_instance;
    static std::mutex m_mutex;

    OnceInstantiated() = default;

    ~OnceInstantiated() = default;

public:
    OnceInstantiated(const OnceInstantiated &) = delete;

    OnceInstantiated &operator=(const OnceInstantiated &) = delete;

    template<typename... Args>
    static SingletonClass &create(Args &&... args) {
        std::lock_guard lock(m_mutex);
        if (m_instance) {
            return *m_instance;
        }
        m_instance = std::shared_ptr<SingletonClass>(new SingletonClass{std::forward<Args>(args)...});
        return *m_instance;
    }

    static SingletonClass &instance() {
        std::lock_guard lock(m_mutex);
        if (!m_instance) {
            throw std::runtime_error("Singleton not created yet");
        }
        return *m_instance;
    }

    static void destroy() {
        std::lock_guard lock(m_mutex);
        m_instance.reset();
    }
};

template<typename SingletonClass>
std::shared_ptr<SingletonClass> OnceInstantiated<SingletonClass>::m_instance;

template<typename SingletonClass>
std::mutex OnceInstantiated<SingletonClass>::m_mutex;
