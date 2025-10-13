#pragma once
#include <filesystem>
#include <thread>
#include <unordered_map>
#include "Observer/Subject.h"
#include "OnceInstantiated/OnceInstantiated.h"

class DirectoriesWatcher : public Subject<>, public OnceInstantiated<DirectoriesWatcher> {
public:
    ~DirectoriesWatcher() override;

    explicit DirectoriesWatcher();

    void reloadPaths(std::vector<std::filesystem::path> paths);

    void watchLoop();

private:
    void subscribeToPaths();

    void clearFds();

    std::atomic<bool> m_running{true};
    std::vector<std::filesystem::path> m_paths;
    std::thread m_watchThread;
    std::atomic<int> m_inotifyFd, m_epollFd;
    std::unordered_map<int, std::filesystem::path> m_watchDescriptors;
};
