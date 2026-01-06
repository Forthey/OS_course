#include "Conn_Mq.h"

#include <mqueue.h>

#include <cerrno>
#include <iostream>
#include <system_error>

#include "Types/Console.h"

Conn_Mq::Conn_Mq(bool isHost, std::uint64_t connId, std::string inQueueName, std::string outQueueName)
    : m_isHost{isHost}
    , m_connId{connId}
    , m_inQueueName{std::move(inQueueName)}
    , m_outQueueName{std::move(outQueueName)} {
    mq_attr attr{};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = bufferMaxSize;

    m_inQueue = mq_open(m_inQueueName.c_str(), O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
    if (m_inQueue == static_cast<mqd_t>(-1)) {
        int saved = errno;
        if (m_isHost) {
            mq_unlink(m_inQueueName.c_str());
            mq_unlink(m_outQueueName.c_str());
        }
        throw std::system_error(saved, std::generic_category(), "mq_open(in) failed");
    }

    m_outQueue = mq_open(m_outQueueName.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK, 0666, &attr);
    if (m_outQueue == static_cast<mqd_t>(-1)) {
        int saved = errno;
        mq_close(m_inQueue);
        if (m_isHost) {
            mq_unlink(m_inQueueName.c_str());
            mq_unlink(m_outQueueName.c_str());
        }
        throw std::system_error(saved, std::generic_category(), "mq_open(out) failed");
    }
}

Conn_Mq::~Conn_Mq() {
    if (m_inQueue != -1) {
        mq_close(m_inQueue);
    }
    if (m_outQueue != -1) {
        mq_close(m_outQueue);
    }
    if (m_isHost) {
        consoleSrv().system(std::format("Closing Message Queue connection for id {}", m_connId));
        mq_unlink(m_inQueueName.c_str());
        mq_unlink(m_outQueueName.c_str());
    }
}

std::expected<BufferType, ConnReadError> Conn_Mq::read() {
    BufferType buffer;
    buffer.resize(bufferMaxSize);

    const ssize_t readNumber = mq_receive(m_inQueue, static_cast<char*>(buffer.data()), bufferMaxSize, nullptr);

    if (readNumber < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return std::unexpected{ConnReadError::NoData};
        }
        return std::unexpected{ConnReadError::TryReadError};
    }

    if (readNumber == 0) {
        return std::unexpected{ConnReadError::Closed};
    }

    buffer.resize(static_cast<size_t>(readNumber));
    return buffer;
}

ConnWriteResult Conn_Mq::write(const BufferType& buffer) {
    const int rc = mq_send(m_outQueue, buffer.data(), buffer.size(), 0);

    if (rc == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return ConnWriteResult::TemporaryNotReady;
        }
        return ConnWriteResult::WriteError;
    }

    return ConnWriteResult::Success;
}
