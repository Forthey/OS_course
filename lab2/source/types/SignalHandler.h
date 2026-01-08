#pragma once

#pragma once

#include <functional>

class SignalHandler {
public:
    using Callback = std::function<void(pid_t, int)>;

    virtual ~SignalHandler() = default;

    virtual void poll() = 0;

    virtual void onRawSignal(pid_t pid, int value) = 0;
};
