#include <iostream>

#include "ChatHostServer/ChatHostServer.h"
#include "NcursesConsole/NcursesConsole.h"
#include "TaskScheduler/SimpleTaskScheduler.h"

void init() {
    ConsoleSrv::console = std::make_unique<NcursesConsole>();
    TaskSchedulerSrv::taskScheduler = std::make_unique<SimpleTaskScheduler>();
}

int main() {
    init();

    ChatHostServer server;
    try {
        server.serve();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
