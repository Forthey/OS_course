#include "YamlConfigLoader.h"

#include <format>
#include <filesystem>
#include <yaml-cpp/yaml.h>

#include "Logger/SystemLogger.h"

YamlConfigLoader::YamlConfigLoader(Logger &logger)
    : m_logger{logger} {
}

std::shared_ptr<Config> YamlConfigLoader::loadData(const std::filesystem::path &filePath) {
    try {
        YAML::Node yamlConfig = YAML::LoadFile(filePath);

        auto config = std::make_shared<Config>();

        if (!yamlConfig["directories"]) {
            m_logger.error(std::format("Could not find \"directories\" in {}", filePath.string()),
                           SystemLogger::SYSTEM);
            return nullptr;
        }

        for (const auto &entry: yamlConfig["directories"]) {
            std::filesystem::path directory = entry.as<std::string>();

            if (!std::filesystem::exists(directory)) {
                m_logger.warn(std::format("Directory {} in file {} does not exist. Is the path relative?",
                                          directory.string(), filePath.string()), SystemLogger::SYSTEM);
                continue;
            }
            if (!std::filesystem::is_directory(directory)) {
                m_logger.warn(std::format("{} in file {} is not a directory", directory.string(), filePath.string()),
                              SystemLogger::SYSTEM);
                continue;
            }
            if (directory.is_relative()) {
                directory = std::filesystem::absolute(directory);
                m_logger.warn(std::format(
                                  "Directory {} in file {} is specified via relative path. This will work once but not on config reload",
                                  directory.string(), filePath.string()), SystemLogger::SYSTEM);
            }

            config->directories.emplace_back(std::move(directory));
        }

        return config;
    } catch (const YAML::BadFile &e) {
        m_logger.error(std::format("Could not load file {}. Error: {}", filePath.string(), e.what()),
                       SystemLogger::SYSTEM);
    } catch (const YAML::ParserException &e) {
        m_logger.error(std::format("Could not parse file {}. Error: {}", filePath.string(), e.what()),
                       SystemLogger::SYSTEM);
    } catch (const YAML::InvalidNode &e) {
        m_logger.error(std::format("Some node in file {} is invalid. Error: {}", filePath.string(), e.what()),
                       SystemLogger::SYSTEM);
    }

    return nullptr;
}
