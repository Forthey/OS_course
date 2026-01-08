#pragma once
#include <expected>

#include "Common.h"

enum class ClientConnType : int {
    Fifo = 1,
    MessageQueue,
    Socket,
};

enum class ConnReadError {
    NoData,
    TryReadError,
    Unknown,
    Closed,
};

enum class ConnWriteResult {
    Success,
    TemporaryNotReady,
    WriteError,
    UnknownError,
};

class Conn {
public:
    virtual ~Conn() = default;

    virtual std::expected<BufferType, ConnReadError> read() = 0;

    virtual ConnWriteResult write(const BufferType& buffer) = 0;

    virtual IdType connId() const = 0;

    static constexpr std::uint64_t bufferMaxSize = 4096;  // 4KВ макс размер сообщения
};
