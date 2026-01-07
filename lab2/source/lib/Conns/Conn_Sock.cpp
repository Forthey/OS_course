#include "Conn_Sock.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <system_error>

#include "Types/Console.h"

Conn_Sock::Conn_Sock(bool isHost, int connId, std::string socketPath)
    : m_isHost{isHost}
    , m_connId{connId}
    , m_socketPath{std::move(socketPath)} {
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    if (m_socketPath.size() >= sizeof(addr.sun_path)) {
        throw std::runtime_error("UNIX socket path is too long");
    }

    std::strncpy(addr.sun_path, m_socketPath.c_str(), sizeof(addr.sun_path) - 1);

    const auto len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + m_socketPath.size() + 1);

    if (isHost) {
        m_listenFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_listenFd < 0) {
            throw std::system_error(errno, std::generic_category(), "socket(AF_UNIX, SOCK_STREAM) failed");
        }

        ::unlink(m_socketPath.c_str());  // старый файл сокета не должен мешать

        if (::bind(m_listenFd, reinterpret_cast<sockaddr*>(&addr), len) < 0) {
            int e = errno;
            ::close(m_listenFd);
            m_listenFd = -1;
            throw std::system_error(e, std::generic_category(), "bind() failed");
        }

        if (::listen(m_listenFd, SOMAXCONN) < 0) {
            int e = errno;
            ::close(m_listenFd);
            m_listenFd = -1;
            throw std::system_error(e, std::generic_category(), "listen() failed");
        }

        int flags = ::fcntl(m_listenFd, F_GETFL, 0);
        if (flags == -1 || ::fcntl(m_listenFd, F_SETFL, flags | O_NONBLOCK) == -1) {
            int e = errno;
            ::close(m_listenFd);
            m_listenFd = -1;
            throw std::system_error(e, std::generic_category(), "fcntl(O_NONBLOCK) on listenFd failed");
        }

        m_sockFd = -1;
    } else {
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) {
            throw std::system_error(errno, std::generic_category(), "socket(AF_UNIX, SOCK_STREAM) failed");
        }

        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), len) < 0) {
            int e = errno;
            ::close(fd);
            throw std::system_error(e, std::generic_category(), "connect() failed");
        }

        int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags == -1 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            int e = errno;
            ::close(fd);
            throw std::system_error(e, std::generic_category(), "fcntl(O_NONBLOCK) failed");
        }

        m_sockFd = fd;
        m_listenFd = -1;
    }
}

Conn_Sock::~Conn_Sock() {
    consoleSrv().system(std::format("Closing socket connection for id {}", m_connId));
    if (m_sockFd >= 0) {
        ::close(m_sockFd);
    }
    if (m_listenFd >= 0) {
        ::close(m_listenFd);
    }
    if (m_isHost && !m_socketPath.empty()) {
        ::unlink(m_socketPath.c_str());
    }
}

std::expected<BufferType, ConnReadError> Conn_Sock::read() {
    if (!tryAccept()) {
        return std::unexpected{ConnReadError::NoData};
    }

    if (m_sockFd < 0) {
        return std::unexpected{ConnReadError::TryReadError};
    }

    if (!m_readInProgress) {
        m_readInProgress = true;
        m_headerDone = 0;
        m_bodyDone = 0;
        m_expectedLength = 0;
        m_lengthNetOrder = 0;
        m_readBuffer.clear();
    }

    auto* headerBytes = reinterpret_cast<char*>(&m_lengthNetOrder);

    while (m_headerDone < sizeof(m_lengthNetOrder)) {
        ssize_t n = recv(m_sockFd, headerBytes + m_headerDone, sizeof(m_lengthNetOrder) - m_headerDone, 0);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return std::unexpected{ConnReadError::NoData};
            }
            m_readInProgress = false;
            return std::unexpected{ConnReadError::TryReadError};
        }

        if (n == 0) {
            m_readInProgress = false;
            return std::unexpected{ConnReadError::Unknown};
        }

        m_headerDone += static_cast<std::size_t>(n);
    }

    // Заголовок полностью считан, вычисляем длину сообщения
    if (m_expectedLength == 0) {
        m_expectedLength = ntohl(m_lengthNetOrder);

        if (m_expectedLength == 0) {
            m_readInProgress = false;
            return std::unexpected{ConnReadError::Unknown};
        }

        if (m_expectedLength > bufferMaxSize) {
            m_readInProgress = false;
            return std::unexpected{ConnReadError::TryReadError};
        }

        m_readBuffer.resize(m_expectedLength);
        m_bodyDone = 0;
    }

    // Читаем тело сообщения длиной m_expectedLength
    char* body = static_cast<char*>(m_readBuffer.data());

    while (m_bodyDone < m_expectedLength) {
        ssize_t n = recv(m_sockFd, body + m_bodyDone, m_expectedLength - m_bodyDone, 0);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return std::unexpected{ConnReadError::NoData};
            }
            m_readInProgress = false;
            return std::unexpected{ConnReadError::TryReadError};
        }

        if (n == 0) {
            m_readInProgress = false;
            return std::unexpected{ConnReadError::Unknown};
        }

        m_bodyDone += static_cast<std::size_t>(n);
    }

    m_readInProgress = false;

    BufferType result = m_readBuffer;
    return result;
}

ConnWriteResult Conn_Sock::write(const BufferType& buffer) {
    if (!tryAccept()) {
        return ConnWriteResult::TemporaryNotReady;
    }

    if (m_sockFd < 0) {
        return ConnWriteResult::WriteError;
    }

    const std::size_t payloadSize = buffer.size();
    if (payloadSize == 0) {
        return ConnWriteResult::Success;
    }

    if (payloadSize > bufferMaxSize) {
        return ConnWriteResult::WriteError;
    }

    uint32_t lenNet = htonl(static_cast<uint32_t>(payloadSize));

    BufferType out;
    out.resize(sizeof(lenNet) + payloadSize);

    char* outData = static_cast<char*>(out.data());
    std::memcpy(outData, &lenNet, sizeof(lenNet));
    std::memcpy(outData + sizeof(lenNet), buffer.data(), payloadSize);

    const char* rawData = outData;
    std::size_t left = out.size();

    while (left > 0) {
        ssize_t writtenNumber = ::send(m_sockFd, rawData, left, 0);

        if (writtenNumber < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return ConnWriteResult::TemporaryNotReady;
            }
            return ConnWriteResult::WriteError;
        }

        if (writtenNumber == 0) {
            return ConnWriteResult::UnknownError;
        }

        left -= static_cast<std::size_t>(writtenNumber);
        rawData += writtenNumber;
    }

    return ConnWriteResult::Success;
}

bool Conn_Sock::tryAccept() {
    if (!m_isHost) {
        return true;
    }
    if (m_sockFd >= 0) {
        return true;
    }
    if (m_listenFd < 0) {
        return false;
    }

    int fd = ::accept4(m_listenFd, nullptr, nullptr, SOCK_NONBLOCK);
    if (fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        }
        throw std::system_error(errno, std::generic_category(), "accept() failed");
    }

    consoleSrv().system(std::format("Client connected, id {}", m_connId));

    ::close(m_listenFd);
    m_listenFd = -1;
    m_sockFd = fd;

    return true;
}
