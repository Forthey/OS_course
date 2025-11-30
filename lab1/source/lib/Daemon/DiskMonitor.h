#pragma once
#include "Daemon.h"
#include "Config/Config.h"
#include "Config/ConfigLoader.h"
#include "Observer/Messages.h"
#include "Observer/Observer.h"
#include "OnceInstantiated/OnceInstantiated.h"
#include "Queue/ThreadSafeQueue.h"

class DiskMonitor : public Daemon, public OnceInstantiated<DiskMonitor>, public Observer {
    friend class OnceInstantiated;

protected:
    DiskMonitor(const std::string &name, std::filesystem::path configPath,
                std::shared_ptr<ConfigLoader<Config> > configLoader, bool isDebugMode = false);

private:
    std::chrono::milliseconds onMainLoopStep() override;

public:
    ~DiskMonitor() override;

    void reloadConfig() override;

    void put(std::shared_ptr<Message> message) override;

private:
    static void handleFileChangedInd(const std::shared_ptr<FileChangedInd> &message);

    std::shared_ptr<Config> m_config;
    const std::filesystem::path m_configPath;
    std::shared_ptr<ConfigLoader<Config> > m_configLoader;
    ThreadSafeQueue<std::shared_ptr<Message> > m_messageQueue;
};
