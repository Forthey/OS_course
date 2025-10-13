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
#include <thread>
#include <utility>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "Logger/SystemLogger.h"

using namespace std::chrono_literals;

void Daemon::stop() {
    SystemLogger::instance().info("Perfmorming daemon default stop");
    m_shouldBeStopped = true;
}

Daemon::Daemon(const std::string &name, bool isDebugMode) : m_name{name}, m_shouldBeStopped{false},
                                                            m_isDebugMode{isDebugMode} {
    m_pidFile = std::filesystem::path{"/var/run/"} / name;
}

int Daemon::run(std::function<void()>&& onDaemonized) {
    SystemLogger::instance().info("Daemon starting up");

    if (!m_isDebugMode) {
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
    }

    onDaemonized();

    while (!m_shouldBeStopped) {
        std::chrono::milliseconds delay = onMainLoopStep();
        if (delay > 0ms) {
            std::this_thread::sleep_for(delay);
        }
    }

    removePidFile();
    SystemLogger::instance().info("Daemon exiting");

    return EXIT_SUCCESS;
}

bool Daemon::daemonize() const {
    pid_t pid;
    if ((pid = fork()) < 0) {
        std::perror("fork");
        return false;
    }
    if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid");
        return false;
    }

    if ((pid = fork()) < 0) {
        perror("fork #2");
        return false;
    }
    if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }

    umask(0);
    if (chdir("/") < 0) {
        SystemLogger::instance().warn(std::format("chdir(\'/\') failed: {}", strerror(errno)));
    }

    if (int fd0 = open("/dev/null", O_RDWR); fd0 < 0) {
        SystemLogger::instance().warn(std::format("open(/dev/null) failed: {}", strerror(errno)));
    }

    return true;
}

void Daemon::upsertDaemon() const {
    std::ifstream pidFile(m_pidFile);
    if (!pidFile.good()) {
        return;
    }

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
        SystemLogger::instance().warn(std::format("Pidfile {} contains invalid pid '{}', removing", m_pidFile.string(),
                                                  line));
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
            SystemLogger::instance().
                    error(std::format("Failed to send SIGTERM to pid {}: {}", oldPid, strerror(errno)));
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
        SystemLogger::instance().error(std::format("Cannot open pidfile {} for writing: {}", m_pidFile.string(),
                                                   strerror(errno)));
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
        SystemLogger::instance().warn(std::format("Failed to remove pidfile {}: {}", m_pidFile.string(),
                                                  strerror(errno)));
    }
}
