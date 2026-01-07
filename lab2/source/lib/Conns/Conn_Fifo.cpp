#include "Conn_Fifo.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>

#include "Types/Console.h"

Conn_Fifo::Conn_Fifo(bool isHost, int connId, std::string fifoReadPath, std::string fifoWritePath)
    : m_isHost{isHost}
    , m_connId{connId}
    , m_fifoReadPath{std::move(fifoReadPath)}
    , m_fifoWritePath{std::move(fifoWritePath)} {
    if (isHost) {
        if (mkfifo(m_fifoReadPath.c_str(), 0666) < 0 && errno != EEXIST) {
            throw std::system_error(errno, std::generic_category(), "mkfifo(read) failed");
        }
        if (mkfifo(m_fifoWritePath.c_str(), 0666) < 0 && errno != EEXIST) {
            unlink(m_fifoReadPath.c_str());
            throw std::system_error(errno, std::generic_category(), "mkfifo(write) failed");
        }
    }

    m_readFd = open(m_fifoReadPath.c_str(), O_RDONLY | O_NONBLOCK);
    if (m_readFd < 0) {
        int saved = errno;
        if (isHost) {
            unlink(m_fifoReadPath.c_str());
            unlink(m_fifoWritePath.c_str());
        }
        throw std::system_error(saved, std::generic_category(), "open(read) failed");
    }

    const auto flag = isHost ? O_RDWR : O_WRONLY;

    m_writeFd = open(m_fifoWritePath.c_str(), flag | O_NONBLOCK);
    if (m_writeFd < 0) {
        int saved = errno;
        close(m_readFd);
        if (isHost) {
            unlink(m_fifoReadPath.c_str());
            unlink(m_fifoWritePath.c_str());
        }
        throw std::system_error(saved, std::generic_category(), "open(write) failed");
    }
}

Conn_Fifo::~Conn_Fifo() {
    if (m_readFd >= 0) {
        close(m_readFd);
    }
    if (m_writeFd >= 0) {
        close(m_writeFd);
    }

    if (m_isHost) {
        consoleSrv().system(std::format("Closing Fifo connection for id {}", m_connId));
        unlink(m_fifoReadPath.c_str());
        unlink(m_fifoWritePath.c_str());
    }
}

std::expected<BufferType, ConnReadError> Conn_Fifo::read() {
    BufferType buffer;
    buffer.resize(bufferMaxSize);

    const ssize_t readNumber = ::read(m_readFd, static_cast<char*>(buffer.data()), bufferMaxSize);

    if (readNumber < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return std::unexpected{ConnReadError::NoData};
        }
        return std::unexpected{ConnReadError::TryReadError};
    }

    if (readNumber == 0 && m_isConnected) {
        return std::unexpected{ConnReadError::Closed};
    }

    if (!m_isConnected) {
        consoleSrv().system(std::format("Client connected, id {}", m_connId));
        m_isConnected = true;
    }

    buffer.resize(static_cast<size_t>(readNumber));
    return buffer;
}

ConnWriteResult Conn_Fifo::write(const BufferType& buffer) {
    const char* rawData = static_cast<const char*>(buffer.data());
    size_t left = buffer.size();

    while (left > 0) {
        const ssize_t writtenNumber = ::write(m_writeFd, rawData, left);

        if (writtenNumber < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return ConnWriteResult::TemporaryNotReady;
            }
            return ConnWriteResult::WriteError;
        }

        if (writtenNumber == 0) {
            return ConnWriteResult::UnknownError;
        }

        left -= static_cast<size_t>(writtenNumber);
        rawData += writtenNumber;
    }

    return ConnWriteResult::Success;
}
