#include <iostream>
#include <ostream>

#include "Daemon/DiskMonitor.h"

int main() {
    if (!DiskMonitor::instance("config.json").run()) {
        std::cerr << "Failed to run disk monitor" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
