#pragma once
#include "Config.h"
#include "ConfigLoader.h"
#include "Logger/Logger.h"
#include "OnceInstaniated/OnceInstantiated.h"

class YamlConfigLoader : public ConfigLoader<Config>, public OnceInstantiated<YamlConfigLoader> {
    friend class OnceInstantiated;
public:
    std::shared_ptr<Config> loadData(const std::string& filename) override;

protected:
    explicit YamlConfigLoader(Logger& logger);
private:
    Logger& m_logger;
};
