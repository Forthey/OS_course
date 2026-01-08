#pragma once
#include <ncurses.h>

#include <atomic>
#include <queue>
#include <shared_mutex>

#include "Console.h"

class NcursesConsole final : public Console {
    enum class Color {
        Default,
        Green,
        Blue,
        Yellow,
    };

    struct Message {
        std::string text;
        Color color;
    };

public:
    ~NcursesConsole() override;

    void start(InputHandler handler) override;

    void step() override;

    void info(std::string_view text) override { log(text, Color::Yellow); }

    void system(std::string_view text) override { log(text, Color::Default); }

    void privateMsg(std::string_view message) override { log(message, Color::Green); }

    void broadcastMsg(std::string_view message) override { log(message, Color::Blue); }

    void log(std::string_view text, Color color);

    void stop() override;

private:
    void init();

    void flushMessages();

    void drawInputLine() const;

    WINDOW* m_logWindow{};
    WINDOW* m_inputWindow{};

    int m_logHeight{};
    int m_logWidth{};

    std::string m_currentInput;
    InputHandler m_handler;

    std::shared_mutex m_queueMtx;
    std::queue<Message> m_messageQueue;
    std::deque<Message> m_messageHistory;
    static constexpr std::size_t m_historyLimit = 20;
};
