#pragma once
#include <functional>
#include <memory>
#include <string>

class Console {
public:
    using InputHandler = std::function<void(const std::string&)>;

    virtual ~Console() = default;

    virtual void run(InputHandler handler) = 0;

    virtual void info(std::string_view text) = 0;

    virtual void system(std::string_view text) = 0;

    virtual void privateMsg(std::string_view message) = 0;

    virtual void broadcastMsg(std::string_view message) = 0;

    virtual void stop() = 0;
};

struct ConsoleSrv {
    static inline std::unique_ptr<Console> console{};
};

inline Console& consoleSrv() { return *ConsoleSrv::console; }
