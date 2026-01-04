#include "MultiSignalHandler.h"

#include "SignalUtils.h"

MultiSignalHandler::MultiSignalHandler(int signo, Callback callback, std::size_t queueSize)
    : m_signo(signo)
    , m_callback(std::move(callback))
    , m_capacity(queueSize)
    , m_queue(nullptr) {
    m_queue = new Event[m_capacity];

    SignalUtils::installHandler(m_signo, this);
}

MultiSignalHandler::~MultiSignalHandler() {
    SignalUtils::uninstallHandler(m_signo, this);
    delete[] m_queue;
}

void MultiSignalHandler::poll() {
    std::size_t head = m_head.load(std::memory_order_relaxed);
    std::size_t tail = m_tail.load(std::memory_order_acquire);

    while (head != tail) {
        Event ev = m_queue[head];
        head = (head + 1) % m_capacity;
        m_head.store(head, std::memory_order_release);

        if (m_callback) {
            m_callback(ev.pid, ev.value);
        }
    }
}

void MultiSignalHandler::onRawSignal(pid_t pid, int value) {
    std::size_t tail = m_tail.load(std::memory_order_relaxed);
    std::size_t next = (tail + 1) % m_capacity;

    std::size_t head = m_head.load(std::memory_order_acquire);
    if (next == head) {
        // очередь полна - дропаем сигнал
        return;
    }

    m_queue[tail] = Event{pid, value};
    m_tail.store(next, std::memory_order_release);
}
