#include <iostream>

#include "CharClientServer/ChatClientServer.h"
#include "NcursesConsole/NcursesConsole.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <server pid>" << std::endl;
        return 1;
    }

    ConsoleSrv::console = std::make_unique<NcursesConsole>();
    ChatClientServer server{std::stoi(argv[1])};

    try {
        server.serve();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
