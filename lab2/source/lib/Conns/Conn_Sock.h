#pragma once
#include "Conn.h"

#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

class Conn_Sock : public Conn {
public:
    Conn_Sock(const std::string& socketPath, int connId);

    ~Conn_Sock() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    int m_sockFd;
    int m_connId;

    bool m_readInProgress{false};
    std::uint32_t m_lengthNetOrder{0};
    std::size_t m_headerDone{0};
    std::uint32_t m_expectedLength{0};
    std::size_t m_bodyDone{0};
    BufferType m_readBuffer;
};
