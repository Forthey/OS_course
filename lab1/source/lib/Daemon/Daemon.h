#pragma once
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class Daemon {
public:
    Daemon(const Daemon &) = delete;

    Daemon &operator=(const Daemon &) = delete;

    virtual ~Daemon() = default;

    int run(std::function<void()>&& onDaemonized);

    virtual void reloadConfig() = 0;

    virtual void stop();

protected:
    explicit Daemon(const std::string &name, bool isDebugMode);

    std::string m_name;

private:
    bool daemonize() const;

    void upsertDaemon() const;

    bool upsertPidFile() const;

    void removePidFile() const;

    virtual std::chrono::milliseconds onMainLoopStep() = 0;

    std::atomic<bool> m_shouldBeStopped;
    std::filesystem::path m_pidFile;
    bool m_isDebugMode;
};
