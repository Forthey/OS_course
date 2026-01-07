#include <iostream>

#include "ChatHostServer/ChatHostServer.h"
#include "NcursesConsole/NcursesConsole.h"

int main() {
    ConsoleSrv::console = std::make_unique<NcursesConsole>();
    ChatHostServer server;
    try {
        server.serve();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
