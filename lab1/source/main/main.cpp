#include <iostream>
#include <ostream>

#include "Config/YamlConfigLoader.h"
#include "Daemon/DiskMonitor.h"
#include "Logger/SystemLogger.h"

void createSingletons() {
    const std::string name = "disk_monitor";
    const std::string configPath = "config.yaml";
    SystemLogger::create(name);
    // На всякий случай уточню: конфиги могут писать в другие логи, по Daemon всегда должен писать в syslog
    // Поэтому здесь я передаю логгер как аргумент, а в DiskMonitor пользуюсь синглтоном напрямую
    YamlConfigLoader::create(SystemLogger::instance());
    DiskMonitor::create(name, configPath);
}

int main() {
    createSingletons();

    const auto data = YamlConfigLoader::instance().loadData("config.yaml");


    if (!DiskMonitor::instance().run()) {
        std::cerr << "Failed to run disk monitor" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
