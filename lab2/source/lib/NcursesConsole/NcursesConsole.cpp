#include "NcursesConsole.h"

#include <locale.h>

#include <cctype>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

static auto* GREEN  = "\033[32m";
static auto* BLUE   = "\033[34m";
static auto* YELLOW = "\033[33m";
static auto* RESET  = "\033[0m";

NcursesConsole::~NcursesConsole() {
    if (m_isRunning.load()) {
        m_isRunning = false;
        endwin();
    }

    if (m_messageHistory.empty()) {
        return;
    }

    std::cout << "\nПоследние сообщения\n";
    for (const auto& [text, color] : m_messageHistory) {
        switch (color) {
            case Color::Green:
                std::cout << GREEN;
                break;
            case Color::Blue:
                std::cout << BLUE;
                break;
            case Color::Yellow:
                std::cout << YELLOW;
                break;
            default:
                break;
        }

        std::cout << text << RESET << "\n";
    }
    std::cout << std::endl;
}

void NcursesConsole::run(InputHandler handler) {
    m_isRunning = true;

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    start_color();
    use_default_colors();

    init_pair(1, COLOR_GREEN,  -1);
    init_pair(2, COLOR_BLUE,   -1);
    init_pair(3, COLOR_YELLOW, -1);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    m_logWidth = cols;
    m_logHeight = rows - 2;

    m_logWindow = newwin(m_logHeight, cols, 0, 0);
    scrollok(m_logWindow, TRUE);

    m_inputWindow = newwin(1, cols, rows - 1, 0);
    nodelay(m_inputWindow, TRUE);

    std::string currentInput;

    while (m_isRunning) {
        flushMessages();
        drawInputLine(currentInput);

        if (const int ch = wgetch(m_inputWindow); ch != ERR) {
            handleKey(ch, currentInput, handler);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    delwin(m_logWindow);
    delwin(m_inputWindow);
    endwin();
}

void NcursesConsole::log(std::string_view text, Color color) {
    std::unique_lock lock{m_queueMtx};
    m_messageHistory.emplace_back(std::string{text}, color);
    if (m_messageHistory.size() > m_historyLimit) {
        m_messageHistory.pop_front();
    }
    m_messageQueue.emplace(std::string{text}, color);
}

void NcursesConsole::stop() { m_isRunning = false; }

void NcursesConsole::flushMessages() {
    std::unique_lock lock{m_queueMtx};
    while (!m_messageQueue.empty()) {
        const auto& [msg, color] = m_messageQueue.front();

        switch (color) {
            case Color::Green:
                wattron(m_logWindow, COLOR_PAIR(1));
                break;
            case Color::Blue:
                wattron(m_logWindow, COLOR_PAIR(2));
                break;
            case Color::Yellow:
                wattron(m_logWindow, COLOR_PAIR(3));
                break;
            default:
                break;
        }

        wprintw(m_logWindow, "%s\n", msg.c_str());
        wattroff(m_logWindow, A_COLOR);

        wrefresh(m_logWindow);
        m_messageQueue.pop();
    }
}

void NcursesConsole::drawInputLine(const std::string& currentInput) const {
    werase(m_inputWindow);
    mvwprintw(m_inputWindow, 0, 0, "> %s", currentInput.c_str());
    wclrtoeol(m_inputWindow);
    wrefresh(m_inputWindow);
}

void NcursesConsole::handleKey(int ch, std::string& currentInput, const InputHandler& handler) const {
    if (ch == '\n' || ch == '\r') {
        std::string line = currentInput;
        currentInput.clear();
        drawInputLine(currentInput);

        if (handler && !line.empty()) {
            handler(line);
        }
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
        if (!currentInput.empty()) currentInput.pop_back();
    } else if (isprint(ch)) {
        currentInput.push_back(static_cast<char>(ch));
    }
}
