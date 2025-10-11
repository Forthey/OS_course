#include "DiskMonitor.h"

#include <sys/syslog.h>

DiskMonitor::DiskMonitor(std::string configPath) : Daemon("disk_monitor") {
}

bool DiskMonitor::onMainLoopStep() {
    syslog(LOG_INFO, "Working...");
    return true;
}
