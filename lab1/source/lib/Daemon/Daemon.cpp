#include "Daemon.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <format>
#include <fstream>
#include <iosfwd>
#include <sstream>
#include <string>
#include <utility>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "Logger/SystemLogger.h"

Daemon::Daemon(const std::string &name) : m_name{name} {
    m_pidFile = std::filesystem::path{"/var/run/"} / name;
}

int Daemon::run() {
    SystemLogger::instance().info("Daemon starting up");

    upsertDaemon();

    if (!daemonize()) {
        SystemLogger::instance().error("Daemonize failed");
        return EXIT_FAILURE;
    }

    SystemLogger::instance().info("Daemonized successfully");

    if (!upsertPidFile()) {
        SystemLogger::instance().error(std::format("Failed to write pid-file {}", m_pidFile.string()));
        return EXIT_FAILURE;
    }

    while (onMainLoopStep()) {
    }

    removePidFile();
    SystemLogger::instance().info("Daemon exiting");

    return EXIT_SUCCESS;
}

bool Daemon::daemonize() const {
    // ... остальной код ...

    if (chdir("/") < 0) {
        SystemLogger::instance().warn(std::format("chdir('/') failed: {}", strerror(errno)));
    }

    if (int fd0 = open("/dev/null", O_RDWR); fd0 < 0) {
        SystemLogger::instance().warn(std::format("open(/dev/null) failed: {}", strerror(errno)));
    }

    return true;
}

void Daemon::upsertDaemon() const {
    std::ifstream pidFile(m_pidFile);
    if (!pidFile.good()) return;

    pidFile.seekg(0);
    std::string line;
    if (!std::getline(pidFile, line)) {
        pidFile.close();
        SystemLogger::instance().warn(std::format("Pidfile {} exists but unreadable, removing", m_pidFile.string()));
        unlink(m_pidFile.c_str());
        return;
    }

    pidFile.close();
    pid_t oldPid = 0;
    try {
        oldPid = static_cast<pid_t>(std::stol(line));
    } catch (...) {
        SystemLogger::instance().warn(std::format("Pidfile {} contains invalid pid '{}', removing", m_pidFile.string(), line));
        unlink(m_pidFile.c_str());
        return;
    }

    if (oldPid <= 0) {
        SystemLogger::instance().warn(std::format("Invalid pid {} in {}, removing", oldPid, m_pidFile.string()));
        unlink(m_pidFile.c_str());
        return;
    }

    std::ostringstream procpath;
    procpath << "/proc/" << oldPid;
    if (access(procpath.str().c_str(), F_OK) == 0) {
        SystemLogger::instance().info(std::format("Found existing process with pid {} (via {}). Sending SIGTERM.",
                                                  oldPid, procpath.str()));
        if (kill(oldPid, SIGTERM) != 0) {
            SystemLogger::instance().error(std::format("Failed to send SIGTERM to pid {}: {}", oldPid, strerror(errno)));
        } else {
            SystemLogger::instance().info(std::format("SIGTERM sent to pid {}", oldPid));
        }
    } else {
        SystemLogger::instance().info(std::format("No process for pid {} (no {}). Removing stale pidfile.",
                                                  oldPid, procpath.str()));
        unlink(m_pidFile.c_str());
    }
}

bool Daemon::upsertPidFile() const {
    const pid_t pid = getpid();
    std::ofstream pidFile(m_pidFile, std::ios::trunc);
    if (!pidFile.is_open()) {
        SystemLogger::instance().error(std::format("Cannot open pidfile {} for writing: {}", m_pidFile.string(), strerror(errno)));
        return false;
    }
    pidFile << pid << std::endl;
    pidFile.close();
    chmod(m_pidFile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    SystemLogger::instance().info(std::format("Wrote pid {} to {}", pid, m_pidFile.string()));
    return true;
}

void Daemon::removePidFile() const {
    if (unlink(m_pidFile.c_str()) == 0) {
        SystemLogger::instance().info(std::format("Removed pidfile {}", m_pidFile.string()));
    } else if (errno != ENOENT) {
        SystemLogger::instance().warn(std::format("Failed to remove pidfile {}: {}", m_pidFile.string(), strerror(errno)));
    }
}
