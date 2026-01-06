#pragma once
#include "Types/Conn.h"

static inline constexpr auto FIFO_TO_HOST_CHANNEL_BASE = "conn_fifo_c2h";
static inline constexpr auto FIFO_TO_CLIENT_CHANNEL_BASE = "conn_fifo_h2c";

class Conn_Fifo : public Conn {
public:
    Conn_Fifo(bool isHost, int connId, std::string fifoReadPath, std::string fifoWritePath);

    ~Conn_Fifo() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    bool m_isHost;
    int m_readFd;
    int m_writeFd;
    int m_connId;
    std::string m_fifoReadPath;
    std::string m_fifoWritePath;
};
