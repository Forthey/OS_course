#pragma once

template<typename T>
class OnceInstantiated {
protected:
    OnceInstantiated() = default;
    ~OnceInstantiated() = default;
public:
    OnceInstantiated(const OnceInstantiated &) = delete;
    OnceInstantiated &operator=(const OnceInstantiated &) = delete;

    template <typename... Args>
    static T &instance(Args&&... args) {
        static T instance{std::forward<Args>(args)...};
        return instance;
    }
};
