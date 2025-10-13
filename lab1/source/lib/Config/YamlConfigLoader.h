#pragma once
#include "Config.h"
#include "ConfigLoader.h"
#include "Logger/Logger.h"
#include "OnceInstantiated/OnceInstantiated.h"

class YamlConfigLoader : public ConfigLoader<Config> {
public:
    explicit YamlConfigLoader(Logger &logger);

    std::shared_ptr<Config> loadData(const std::filesystem::path &filename) override;

private:
    Logger &m_logger;
};
