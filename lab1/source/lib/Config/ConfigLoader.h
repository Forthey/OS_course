#pragma once
#include <memory>
#include <filesystem>

template<typename Data>
class ConfigLoader {
public:
    virtual ~ConfigLoader() = default;

    virtual std::shared_ptr<Data> loadData(const std::filesystem::path &filename) = 0;
};
