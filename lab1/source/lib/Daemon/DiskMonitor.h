#pragma once
#include "Daemon.h"
#include "OnceInstantiated/OnceInstantiated.h"

class DiskMonitor : public Daemon, public OnceInstantiated<DiskMonitor> {
    friend class OnceInstantiated;
protected:
    DiskMonitor(const std::string& name, const std::string& configPath);

private:
    bool onMainLoopStep() override;
};
