#include "ChatHostServer/ChatHostServer.h"
#include "NcursesConsole/NcursesConsole.h"

int main() {
    ConsoleSrv::console = std::make_unique<NcursesConsole>();
    ChatHostServer server;
    server.serve();
    return 0;
}
