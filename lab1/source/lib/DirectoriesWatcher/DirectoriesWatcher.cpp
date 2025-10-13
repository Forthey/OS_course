#include "DirectoriesWatcher.h"

#include <cstring>
#include <format>
#include <ranges>
#include <sys/inotify.h>
#include <sys/epoll.h>

#include "Logger/SystemLogger.h"
#include "Observer/Messages.h"

namespace {
    FileChangedInd::Action getActionByMask(std::uint32_t mask) {
        if (mask & IN_MODIFY) {
            return FileChangedInd::Action::MODIFIED;
        }
        if (mask & IN_DELETE) {
            return FileChangedInd::Action::CREATED;
        }
        return FileChangedInd::Action::CREATED;
    }
}

DirectoriesWatcher::~DirectoriesWatcher() {
    m_running = false;
    m_watchThread.join();
    clearFds();
}

DirectoriesWatcher::DirectoriesWatcher() {
    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFd < 0) {
        perror("inotify_init1");
        return;
    }

    // Создаём epoll
    m_epollFd = epoll_create1(0);
    if (m_epollFd < 0) {
        perror("epoll_create1");
        clearFds();
        return;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = m_inotifyFd;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_inotifyFd, &event) < 0) {
        perror("epoll_ctl");
        clearFds();
        return;
    }

    m_watchThread = std::thread{&DirectoriesWatcher::watchLoop, this};
}

void DirectoriesWatcher::reloadPaths(std::vector<std::filesystem::path> paths) {
    m_paths = std::move(paths);
    for (const auto &wd: std::views::keys(m_watchDescriptors)) {
        inotify_rm_watch(m_inotifyFd, wd);
    }
    subscribeToPaths();
}

void DirectoriesWatcher::subscribeToPaths() {
    for (const auto &dir: m_paths) {
        int wd = inotify_add_watch(m_inotifyFd, dir.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
        if (wd < 0) {
            SystemLogger::instance().error(std::format("Cannot watch directory {}: {}", dir.string(),
                                                       std::strerror(errno)));
            continue;
        }
        m_watchDescriptors[wd] = dir;

        SystemLogger::instance().info(std::format("Observing directory {}", dir.string()));
    }
}

void DirectoriesWatcher::watchLoop() {
    constexpr int MAX_EVENTS = 10;
    epoll_event events[MAX_EVENTS];

    while (m_running) {
        const int n = epoll_wait(m_epollFd, events, MAX_EVENTS, 500); // 500ms таймаут
        if (n < 0 && errno != EINTR) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == m_inotifyFd) {
                char buffer[1024];
                const long length = read(m_inotifyFd, buffer, sizeof(buffer));
                if (length < 0) {
                    if (errno != EAGAIN) {
                        perror("read");
                    }
                    continue;
                }

                long offset = 0;
                while (offset < length) {
                    auto *eventPtr = reinterpret_cast<inotify_event *>(buffer + offset);
                    if (eventPtr->len) {
                        const std::string file{eventPtr->name};
                        const auto path = m_watchDescriptors[eventPtr->wd];
                        notify(std::shared_ptr<FileChangedInd>{
                            new FileChangedInd{file, getActionByMask(eventPtr->mask)}
                        });
                    }
                    offset += sizeof(inotify_event) + eventPtr->len;
                }
            }
        }
    }

    clearFds();
}

void DirectoriesWatcher::clearFds() {
    for (const auto &wd: std::views::keys(m_watchDescriptors)) {
        inotify_rm_watch(m_inotifyFd, wd);
    }
    close(m_inotifyFd);
    close(m_epollFd);
}
