#include "Daemon.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <sstream>
#include <string>
#include <utility>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/file.h>

Daemon::Daemon(const std::string &name) : m_pidFile(std::filesystem::path("/var/run/") / name), m_name(name) {
}

int Daemon::run() {
    openlog(m_name.c_str(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon starting up");

    upsertDaemon();

    if (!daemonize()) {
        syslog(LOG_ERR, "Daemonize failed");
        closelog();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "Daemonized successfully");

    if (!upsertPidFile()) {
        syslog(LOG_ERR, "Failed to write pid-file %s", m_pidFile.c_str());
        closelog();
        return EXIT_FAILURE;
    }

    while (onMainLoopStep()) {
    }

    removePidFile();
    syslog(LOG_INFO, "Daemon exiting");
    closelog();

    return EXIT_SUCCESS;
}

bool Daemon::daemonize() {
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
        syslog(LOG_WARNING, "chdir('/') failed: %s", strerror(errno));
    }

    if (int fd0 = open("/dev/null", O_RDWR); fd0 >= 0) {
        dup2(fd0, STDIN_FILENO);
        dup2(fd0, STDOUT_FILENO);
        dup2(fd0, STDERR_FILENO);
        if (fd0 > 2) {
            close(fd0);
        }
    } else {
        syslog(LOG_WARNING, "open(/dev/null) failed: %s", strerror(errno));
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
        syslog(LOG_WARNING, "Pidfile %s exists but unreadable, removing", m_pidFile.c_str());
        unlink(m_pidFile.c_str());
        return;
    }

    pidFile.close();
    pid_t oldPid = 0;
    try {
        oldPid = static_cast<pid_t>(std::stol(line));
    } catch (...) {
        syslog(LOG_WARNING, "Pidfile %s contains invalid pid '%s', removing", m_pidFile.c_str(), line.c_str());
        unlink(m_pidFile.c_str());
        return;
    }

    if (oldPid <= 0) {
        syslog(LOG_WARNING, "Invalid pid %d in %s, removing", oldPid, m_pidFile.c_str());
        unlink(m_pidFile.c_str());
        return;
    }

    std::ostringstream procpath;
    procpath << "/proc/" << oldPid;
    if (access(procpath.str().c_str(), F_OK) == 0) {
        syslog(LOG_INFO, "Found existing process with pid %d (via %s). Sending SIGTERM.", oldPid,
               procpath.str().c_str());
        if (kill(oldPid, SIGTERM) != 0) {
            syslog(LOG_ERR, "Failed to send SIGTERM to pid %d: %s", oldPid, strerror(errno));
        } else {
            syslog(LOG_INFO, "SIGTERM sent to pid %d", oldPid);
        }
    } else {
        syslog(LOG_INFO, "No process for pid %d (no %s). Removing stale pidfile.", oldPid, procpath.str().c_str());
        unlink(m_pidFile.c_str());
    }
}

bool Daemon::upsertPidFile() const {
    const pid_t pid = getpid();
    std::ofstream pidFile(m_pidFile, std::ios::trunc);
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Cannot open pidfile %s for writing: %s", m_pidFile.c_str(), strerror(errno));
        return false;
    }
    pidFile << pid << std::endl;
    pidFile.close();
    chmod(m_pidFile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    syslog(LOG_INFO, "Wrote pid %d to %s", pid, m_pidFile.c_str());
    return true;
}

void Daemon::removePidFile() const {
    if (unlink(m_pidFile.c_str()) == 0) {
        syslog(LOG_INFO, "Removed pidfile %s", m_pidFile.c_str());
    } else {
        if (errno != ENOENT)
            syslog(LOG_WARNING, "Failed to remove pidfile %s: %s", m_pidFile.c_str(), strerror(errno));
    }
}
