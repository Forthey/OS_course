#pragma once
#include <expected>

#include "Types.h"

class Conn {
public:
    virtual ~Conn() = default;

    virtual std::expected<BufferType, ConnReadError> read() = 0;

    virtual ConnWriteResult write(const BufferType& buffer) = 0;

    virtual IdType connId() const = 0;

    static constexpr std::uint64_t bufferMaxSize = 1024 * 1024;  // 1МВ макс размер сообщения
};
