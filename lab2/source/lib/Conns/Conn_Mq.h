#pragma once
#include <mqueue.h>

#include "Conn.h"

class ConnMq : public Conn {
public:
    ConnMq(bool isHost, std::string inQueueName, std::string outQueueName, std::uint64_t connId, size_t messageSize);

    ~ConnMq() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    bool m_isHost;
    std::string m_inQueueName;
    std::string m_outQueueName;
    mqd_t m_inQueue;
    mqd_t m_outQueue;
    std::uint64_t m_connId;
    std::size_t m_messageSize;
};
