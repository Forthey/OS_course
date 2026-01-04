#pragma once
#include <chrono>

using BufferType = std::string;
using IdType = std::size_t;
using TimestampType = std::chrono::system_clock::time_point;

static inline constexpr auto MQ_TO_HOST_CHANNEL_BASE = "conn_mq_c2h";
static inline constexpr auto MQ_TO_CLIENT_CHANNEL_BASE = "conn_mq_h2c";

static inline constexpr auto FIFO_TO_HOST_CHANNEL_BASE = "conn_fifo_c2h";
static inline constexpr auto FIFO_TO_CLIENT_CHANNEL_BASE = "conn_fifo_h2c";

static inline constexpr auto SOCKET_CHANNEL_BASE = "conn_socket";

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
