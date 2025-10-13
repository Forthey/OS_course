#include <iostream>
#include <ostream>

#include "Config/YamlConfigLoader.h"
#include "Daemon/DiskMonitor.h"
#include "DirectoriesWatcher/DirectoriesWatcher.h"
#include "Logger/SystemLogger.h"
#include "SignalHandler/SignalHandler.h"

/// Если задана, программа будет запускаться как программа, а не как демон
// #define DEBUG_MOD

void onDaemonized() {
    DirectoriesWatcher::create();
    DirectoriesWatcher::instance().attach(&DiskMonitor::instance());
    SignalHandler::instance().attach(&DiskMonitor::instance());

    DiskMonitor::instance().put(std::make_shared<ReloadConfigRequest>());
}

bool runDaemon(const std::filesystem::path &configPath) {
    const std::string name = "disk_monitor";
    SystemLogger::create(name);
    SignalHandler::create();

#ifndef DEBUG_MOD
    DiskMonitor::create(name, configPath, std::make_shared<YamlConfigLoader>());
#else
    DiskMonitor::create(name, configPath, std::make_shared<YamlConfigLoader>(), true);
#endif // DEBUG_MOD
    if (!DiskMonitor::instance().run(onDaemonized)) {
        std::cerr << "Failed to run disk monitor" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void deleteSingletons() {
    SignalHandler::destroy();
    DiskMonitor::destroy();
    SystemLogger::destroy();
    DirectoriesWatcher::destroy();
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [config.yaml]" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path configPath = argc == 2 ? argv[1] : "config.yaml";
    if (!runDaemon(std::filesystem::absolute(configPath))) {
        deleteSingletons();
        return EXIT_FAILURE;
    }
    deleteSingletons();
    return EXIT_SUCCESS;
}
