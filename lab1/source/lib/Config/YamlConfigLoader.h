#pragma once
#include "Config.h"
#include "ConfigLoader.h"
#include "OnceInstantiated/OnceInstantiated.h"

class YamlConfigLoader : public ConfigLoader<Config> {
public:
    std::shared_ptr<Config> loadData(const std::filesystem::path &filename) override;
};
