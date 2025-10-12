#pragma once
#include <filesystem>
#include <memory>
#include <string>

#include "Logger/Logger.h"

class Daemon {
public:
    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;
    virtual ~Daemon() = default;

    int run();
protected:
    Daemon(const std::string& name);

    std::string m_name;
private:
    bool daemonize() const;
    void upsertDaemon() const;
    bool upsertPidFile() const;
    void removePidFile() const;

    virtual bool onMainLoopStep() = 0;

    std::filesystem::path m_pidFile;
};
