#include "MultiSignalHandler.h"

#include "SignalUtils.h"

MultiSignalHandler::MultiSignalHandler(int signal, Callback callback, std::size_t queueSize)
    : m_signal(signal)
    , m_callback(std::move(callback))
    , m_capacity(queueSize)
    , m_queue(nullptr) {
    m_queue = new Event[m_capacity];

    SignalUtils::installHandler(m_signal, this);
}

MultiSignalHandler::~MultiSignalHandler() {
    SignalUtils::uninstallHandler(m_signal, this);
    delete[] m_queue;
}

void MultiSignalHandler::poll() {
    std::size_t head = m_head;
    const std::size_t tail = m_tail;

    while (head != tail) {
        const auto event = m_queue[head];
        head = (head + 1) % m_capacity;
        m_head = head;

        if (m_callback) {
            m_callback(event.pid, event.value);
        }
    }
}

void MultiSignalHandler::onRawSignal(pid_t pid, int data) {
    const std::size_t tail = m_tail;
    const std::size_t next = (tail + 1) % m_capacity;

    if (next == m_head) {
        // очередь полна - дропаем сигнал
        return;
    }

    m_queue[tail] = Event{pid, data};
    m_tail = next;
}
