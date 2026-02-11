#include <iostream>
#include "udp_server.hpp"

int main(int argc, char** argv) {
    std::cout << "EPH v2.1 GUI Server" << std::endl;
    std::cout << "Initializing UDP server..." << std::endl;

    eph::udp::UDPServer server(5555, 5556);

    if (!server.is_initialized()) {
        std::cerr << "Failed to initialize UDP server: "
                  << server.get_last_error() << std::endl;
        return 1;
    }

    std::cout << "UDP server initialized successfully" << std::endl;
    std::cout << "  Send port: 5555 (state data)" << std::endl;
    std::cout << "  Recv port: 5556 (commands)" << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    // TODO: Integrate with SwarmManager in next task

    while (true) {
        auto command = server.receive_command();
        if (command.has_value()) {
            std::cout << "Received command: " << command.value().dump() << std::endl;
        }

        usleep(100000); // 100ms
    }

    return 0;
}
