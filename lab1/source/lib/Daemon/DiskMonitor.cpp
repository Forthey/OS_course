#include "DiskMonitor.h"

#include <cstring>
#include <format>
#include <ranges>
#include <utility>

#include "DirectoriesWatcher/DirectoriesWatcher.h"
#include "../Observer/Messages.h"
#include "Logger/SystemLogger.h"
#include "SignalHandler/SignalHandler.h"

using namespace std::chrono_literals;

DiskMonitor::DiskMonitor(const std::string &name, std::filesystem::path configPath,
                         std::shared_ptr<ConfigLoader<Config> > configLoader,
                         const bool isDebugMode) : Daemon{name, isDebugMode},
                                                   m_configPath{std::move(configPath)},
                                                   m_configLoader{std::move(configLoader)} {
    DiskMonitor::reloadConfig();
    DirectoriesWatcher::instance().attach(this);
    SignalHandler::instance().attach(this);
}

std::chrono::milliseconds DiskMonitor::onMainLoopStep() {
    const auto message = m_messageQueue.pop();
    SystemLogger::instance().info("Processing message...");
    if (const auto fileChangedInd = std::dynamic_pointer_cast<FileChangedInd>(message)) {
        handleFileChangedInd(fileChangedInd);
    } else if (const auto reloadConfigRequest = std::dynamic_pointer_cast<ReloadConfigRequest>(message)) {
        reloadConfig();
    } else if (const auto stopRequest = std::dynamic_pointer_cast<StopRequest>(message)) {
        stop();
    } else {
        SystemLogger::instance().error("Unexpected message type in DiskMonitor::update");
    }
    return 0ms;
}

DiskMonitor::~DiskMonitor() = default;

void DiskMonitor::reloadConfig() {
    m_config = m_configLoader->loadData(m_configPath);
    if (!m_config) {
        perror("Could not load config");
        return;
    }
    SystemLogger::instance().info("Config loaded successfully");
    DirectoriesWatcher::instance().reloadPaths(m_config->directories);
}

void DiskMonitor::put(std::shared_ptr<Message> message) {
    SystemLogger::instance().info("Processing message...");
    m_messageQueue.push(message);
}

void DiskMonitor::handleFileChangedInd(const std::shared_ptr<FileChangedInd> &message) {
    switch (message->action) {
        case FileChangedInd::Action::CREATED:
            SystemLogger::instance().info(std::format("Created file {}", message->fileName), SystemLogger::LOCAL0);
            break;
        case FileChangedInd::Action::DELETED:
            SystemLogger::instance().info(std::format("Deleted file {}", message->fileName), SystemLogger::LOCAL0);
            break;
        case FileChangedInd::Action::MODIFIED:
            SystemLogger::instance().info(std::format("Modified file {}", message->fileName), SystemLogger::LOCAL0);
            break;
    }
}
