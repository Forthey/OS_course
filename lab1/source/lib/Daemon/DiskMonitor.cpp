#include "DiskMonitor.h"

#include <cstring>
#include <format>
#include <ranges>
#include <utility>

#include "DirectoriesWatcher/DirectoriesWatcher.h"
#include "Observer/Messages.h"
#include "Logger/SystemLogger.h"

using namespace std::chrono_literals;

DiskMonitor::DiskMonitor(const std::string &name, std::filesystem::path configPath,
                         std::shared_ptr<ConfigLoader<Config> > configLoader,
                         const bool isDebugMode) : Daemon{name, isDebugMode},
                                                   m_configPath{std::move(configPath)},
                                                   m_configLoader{std::move(configLoader)} {
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
    std::string strAction;
    std::string type;

    if (std::filesystem::is_directory(message->directory / message->fileName)) {
        type = "directory";
    } else {
        type = "file";
    }

    switch (message->action) {
        case FileChangedInd::Action::CREATED:
            strAction = "Created";
            break;
        case FileChangedInd::Action::DELETED:
            strAction = "Deleted";
            break;
        case FileChangedInd::Action::MODIFIED:
            strAction = "Modified";
            break;
    }

    SystemLogger::instance().info(std::format("{} {} {} in directory {}",
                                              strAction, type, message->fileName, message->directory.string()),
                                  SystemLogger::LOCAL0);
}
