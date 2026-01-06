#pragma once
#include <mqueue.h>

#include "Types/Conn.h"

static inline constexpr auto MQ_TO_HOST_CHANNEL_BASE = "/conn_mq_c2h";
static inline constexpr auto MQ_TO_CLIENT_CHANNEL_BASE = "/conn_mq_h2c";

class Conn_Mq : public Conn {
public:
    Conn_Mq(bool isHost, std::uint64_t connId, std::string inQueueName, std::string outQueueName);

    ~Conn_Mq() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    bool m_isHost;
    std::uint64_t m_connId;
    std::string m_inQueueName;
    std::string m_outQueueName;
    mqd_t m_inQueue;
    mqd_t m_outQueue;
};
