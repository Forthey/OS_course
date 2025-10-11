#pragma once
#include "Daemon.h"
#include "OnceInstaniated/OnceInstantiated.h"

class DiskMonitor : public Daemon, public OnceInstantiated<DiskMonitor> {
    friend class OnceInstantiated;
protected:
    explicit DiskMonitor(std::string configPath = "");

private:
    bool onMainLoopStep() override;
};
