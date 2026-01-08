#pragma once
#include <chrono>
#include <functional>

class TaskScheduler {
    using Task = std::function<void()>;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

public:
    virtual ~TaskScheduler() = default;

    virtual void addStopCallback(Task cb) = 0;

    virtual void scheduleRepeated(Task task, Duration sleepTime) = 0;

    virtual void schedule(Task task) = 0;

    virtual void stop() = 0;

    virtual void wait() = 0;
};

struct TaskSchedulerSrv {
    static inline std::unique_ptr<TaskScheduler> taskScheduler{};
};

inline TaskScheduler& taskSchedulerSrv() { return *TaskSchedulerSrv::taskScheduler; }
