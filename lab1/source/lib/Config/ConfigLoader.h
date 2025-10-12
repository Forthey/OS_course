#pragma once
#include <memory>

template<typename Data>
class ConfigLoader {
public:
    virtual ~ConfigLoader() = default;
    virtual std::shared_ptr<Data> loadData(const std::string& filename) = 0;
};
