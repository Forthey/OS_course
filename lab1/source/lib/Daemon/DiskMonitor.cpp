#include "DiskMonitor.h"

#include "Logger/SystemLogger.h"

DiskMonitor::DiskMonitor(const std::string &name,
                         const std::string &configPath) : Daemon{name} {
}

bool DiskMonitor::onMainLoopStep() {
    SystemLogger::instance().info("Working...");
    sleep(1);
    return true;
}
