#include "Conn_Sock.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <system_error>

Conn_Sock::Conn_Sock(const std::string& socketPath, int connId) : m_connId{connId} {
    m_sockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_sockFd < 0) {
        throw std::runtime_error("socket(AF_UNIX, SOCK_STREAM) failed");
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    if (socketPath.size() >= sizeof(addr.sun_path)) {
        close(m_sockFd);
        throw std::runtime_error("UNIX socket path is too long");
    }

    std::strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    const auto len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + socketPath.size() + 1);

    if (connect(m_sockFd, reinterpret_cast<sockaddr*>(&addr), len) < 0) {
        const int savedErrno = errno;
        close(m_sockFd);
        throw std::system_error(savedErrno, std::generic_category(), "connect() failed");
    }

    int flags = fcntl(m_sockFd, F_GETFL, 0);
    if (flags == -1 || ::fcntl(m_sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        int saved = errno;
        close(m_sockFd);
        throw std::system_error(saved, std::generic_category(), "fcntl(O_NONBLOCK) failed");
    }
}

Conn_Sock::~Conn_Sock() {
    if (m_sockFd >= 0) {
        close(m_sockFd);
    }
}

std::expected<BufferType, ConnReadError> Conn_Sock::read() {
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
    const char* rawData = static_cast<const char*>(buffer.data());
    std::size_t left = buffer.size();

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
