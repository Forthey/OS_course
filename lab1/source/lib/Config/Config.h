#pragma once
#include <filesystem>
#include <vector>

struct Config {
    std::vector<std::filesystem::path> directories;
};