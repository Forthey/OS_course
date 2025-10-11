#pragma once
#include <string>

class Daemon {
public:
    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;
    virtual ~Daemon() = default;

    int run();
protected:
    explicit Daemon(const std::string &name);
private:
    static bool daemonize();
    void upsertDaemon() const;
    bool upsertPidFile() const;
    void removePidFile() const;

    virtual bool onMainLoopStep() = 0;

    std::string m_pidFile;
    std::string m_name;
};
