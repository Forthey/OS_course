#include "SingleSignalHandler.h"

#include <iostream>

#include "SignalUtils.h"

SingleSignalHandler::SingleSignalHandler(int signo, Callback callback)
    : m_signo{signo}
    , m_callback{std::move(callback)} {
    SignalUtils::installHandler(m_signo, this);
}

SingleSignalHandler::~SingleSignalHandler() { SignalUtils::uninstallHandler(m_signo, this); }

void SingleSignalHandler::poll() {
    if (m_hasEvent.exchange(false, std::memory_order_acq_rel)) {
        pid_t pid = m_lastPid.load(std::memory_order_acquire);
        int value = m_lastValue.load(std::memory_order_acquire);

        if (m_callback) {
            m_callback(pid, value);
        }
    }
}

void SingleSignalHandler::onRawSignal(pid_t pid, int data) {
    m_lastPid = pid;
    m_lastValue = data;
    m_hasEvent = true;
}
