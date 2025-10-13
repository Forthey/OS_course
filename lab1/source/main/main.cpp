#include <iostream>
#include <ostream>

#include "Config/YamlConfigLoader.h"
#include "Daemon/DiskMonitor.h"
#include "DirectoriesWatcher/DirectoriesWatcher.h"
#include "Logger/SystemLogger.h"
#include "SignalHandler/SignalHandler.h"

/// Если задана, программа будет запускаться как программа, а не как демон
// #define DEBUG_MOD

void createSingletons(const std::filesystem::path& configPath) {
    const std::string name = "disk_monitor";
    SystemLogger::create(name);
    DirectoriesWatcher::create();
    SignalHandler::create();
    // На всякий случай уточню: конфиги могут писать в другие логи, по Daemon всегда должен писать в syslog
    // Поэтому в Yaml... передаю логгер как аргумент, а в DiskMonitor пользуюсь синглтоном напрямую
#ifndef DEBUG_MOD
    DiskMonitor::create(name, configPath, std::make_shared<YamlConfigLoader>(SystemLogger::instance()));
#else
    DiskMonitor::create(name, configPath, std::make_shared<YamlConfigLoader>(SystemLogger::instance()), true);
#endif // DEBUG_MOD
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
    createSingletons(std::filesystem::absolute(configPath));
    if (!DiskMonitor::instance().run()) {
        std::cerr << "Failed to run disk monitor" << std::endl;
        return EXIT_FAILURE;
    }
    deleteSingletons();
    return EXIT_SUCCESS;
}
