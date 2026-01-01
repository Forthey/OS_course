#pragma once
#include <chrono>

using BufferType = std::string;
using IdType = std::size_t;
using TimestampType = std::chrono::system_clock::time_point;

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
