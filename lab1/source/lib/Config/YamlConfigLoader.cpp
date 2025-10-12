#include "YamlConfigLoader.h"

#include <format>
#include <filesystem>
#include <yaml-cpp/yaml.h>

YamlConfigLoader::YamlConfigLoader(Logger& logger)
    : m_logger{logger} {
}

std::shared_ptr<Config> YamlConfigLoader::loadData(const std::string &filename) {
    try {
        YAML::Node yamlConfig = YAML::LoadFile(filename);

        auto config = std::make_shared<Config>();

        if (!yamlConfig["directories"]) {
            m_logger.error(std::format("Could not find \"directories\" in {}", filename));
            return nullptr;
        }

        for (const auto &entry : yamlConfig["directories"]) {
            std::filesystem::path directory = entry.as<std::string>();

            if (!std::filesystem::exists(directory)) {
                m_logger.warn(std::format("Directory {} in file {} does not exist", directory.string(), filename));
                continue;
            }
            if (!std::filesystem::is_directory(directory)) {
                m_logger.warn(std::format("{} in file {} is not a directory", directory.string(), filename));
                continue;
            }
            if (directory.is_relative()) {
                m_logger.warn(std::format("Directory {} in file {} is specified via relative path. Are you sure?", directory.string(), filename));
            }

            config->directories.emplace_back(std::move(directory));
        }

        return config;
    } catch (const YAML::BadFile &) {
        m_logger.error(std::format("Could not load file {}", filename));
    } catch (const YAML::ParserException &e) {
        m_logger.error(std::format("Could not parse file {}. Error: {}", filename, e.what()));
    } catch (const YAML::InvalidNode &e) {
        m_logger.error(std::format("Some node in file {} is invalid. Error: {}", filename, e.what()));
    }

    return nullptr;
}
