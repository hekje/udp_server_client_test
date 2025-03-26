/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#include "udp_client_tester.hpp"
#include "sl_log.hpp"
#include <csignal>
#include <cstdlib>
#include <limits>
#include <regex>

volatile bool running = true;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

// Function to validate IP address (IPv4 only)
bool isValidIpAddress(const std::string& ipAddress) {
    const std::regex ipPattern(
        "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
    return std::regex_match(ipAddress, ipPattern);
}

int main(int argc, char* argv[]) {
    // Check if the user provided a port number and IP address
    if (argc != 3) {
        hek::SLLog::LogError("Usage: " + std::string(argv[0]) + " <port> <ipAddress>");
        return EXIT_FAILURE;
    }

    // Convert input port to integer and validate
    char* end;
    long port = std::strtol(argv[1], &end, 10);
    if (*end != '\0' || port <= 0 || port > std::numeric_limits<uint16_t>::max()) {
        hek::SLLog::LogError("Invalid port number. Please provide a valid port (1-65535), eg 8080");
        return EXIT_FAILURE;
    }

    // Validate the provided IP address
    std::string ipAddress = argv[2];
    if (!isValidIpAddress(ipAddress)) {
        hek::SLLog::LogError("Invalid IP address: " + ipAddress + ". Please provide a valid IPv4 address.");
        return EXIT_FAILURE;
    }

    // Register signal handler for CTRL-C
    std::signal(SIGINT, signalHandler);

    // Create the client with the provided IP address and port
    hek::UdpClientTester client(static_cast<uint16_t>(port), ipAddress);

    hek::SLLog::LogInfo("Started UDP Server on IP " + ipAddress + " and port " + std::to_string(port) + ". Press CTRL-C to stop.");

    // Main loop to keep the program running
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    hek::SLLog::LogInfo("Stopping UDP Server...");

    return EXIT_SUCCESS;
}


