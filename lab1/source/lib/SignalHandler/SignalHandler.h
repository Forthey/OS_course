#pragma once
#include "Observer/Subject.h"
#include "OnceInstantiated/OnceInstantiated.h"

class SignalHandler : public Subject<>, public OnceInstantiated<SignalHandler> {
public:
    explicit SignalHandler();

    void handleSignal(int signal);
};
