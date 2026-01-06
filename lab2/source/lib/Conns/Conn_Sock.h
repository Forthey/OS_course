#pragma once
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Types/Conn.h"

static inline constexpr auto SOCKET_CHANNEL_BASE = "conn_socket";

class Conn_Sock : public Conn {
public:
    Conn_Sock(bool isHost, int connId, std::string socketPath);

    ~Conn_Sock() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    bool tryAccept();

    bool m_isHost;
    int m_connId;

    int m_sockFd;
    int m_listenFd;
    std::string m_socketPath;

    bool m_readInProgress{false};
    std::uint32_t m_lengthNetOrder{0};
    std::size_t m_headerDone{0};
    std::uint32_t m_expectedLength{0};
    std::size_t m_bodyDone{0};
    BufferType m_readBuffer;
};
