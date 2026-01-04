#pragma once

#pragma once

#include <sys/types.h>

#include <functional>

class ISignalHandler {
public:
    using Callback = std::function<void(pid_t, int)>;

    virtual ~ISignalHandler() = default;

    virtual void poll() = 0;

    virtual void onRawSignal(pid_t pid, int value) = 0;
};
