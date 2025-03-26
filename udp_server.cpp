/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#include "udp_server_tester.hpp"
#include "sl_log.hpp"
#include <csignal>
#include <cstdlib>
#include <limits>

volatile bool running = true;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

int main(int argc, char* argv[]) {
    // Check if the user provided a port number
    if (argc != 2) {
        hek::SLLog::LogError("Usage: " + std::string(argv[0]) + " <port>");
        return EXIT_FAILURE;
    }

    // Convert input to integer and validate
    char* end;
    long port = std::strtol(argv[1], &end, 10);
    if (*end != '\0' || port <= 0 || port > std::numeric_limits<uint16_t>::max()) {
        hek::SLLog::LogError("Invalid port number. Please provide a valid port (1-65535), eg 8080");
        return EXIT_FAILURE;
    }

    // Register signal handler for CTRL-C
    std::signal(SIGINT, signalHandler);

    hek::UdpServerTester observer(static_cast<uint16_t>(port));

    hek::SLLog::LogInfo("Started UDP Server on port " + std::to_string(port) + ". Press CTRL-C to stop.");

    // Main loop to keep the program running
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    hek::SLLog::LogInfo("Stopping UDP Server...");

    return EXIT_SUCCESS;
}
