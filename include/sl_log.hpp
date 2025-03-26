/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace hek {

class SLLog {
public:
    static std::string GetTimeStamp() {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        std::string timestamp(30, '\0');
        std::strftime(&timestamp[0], timestamp.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        // Get the milliseconds
        int msec = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
                       .time_since_epoch()
                       .count() % 100;
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << msec;
        timestamp += ":" + ss.str();

        return timestamp;
    }

    static void LogInfo(const std::string &inMsg) {
        std::cout << "\033[1;32m" // Green color
                  << GetTimeStamp() << " INFO - " << inMsg.c_str()
                  << "\033[0m"    // Reset color
                  << std::endl << std::flush;
    }

    static void LogWarn(const std::string &inMsg) {
        std::cerr << "\033[1;33m" // Yellow (close to orange)
                  << GetTimeStamp() << " WARNING - " << inMsg.c_str()
                  << "\033[0m"    // Reset color
                  << std::endl << std::flush;
    }

    static void LogError(const std::string &inMsg) {
        std::cerr << "\033[1;31m" // Red color
                  << GetTimeStamp() << " ERROR - " << inMsg.c_str()
                  << "\033[0m"    // Reset color
                  << std::endl << std::flush;
    }

private:
    explicit SLLog();
    ~SLLog();
};

} // namespace hek
