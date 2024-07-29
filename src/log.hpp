#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <string>
#include <chrono>

namespace engine
{
    static const std::string LOG_FILE = "C:\\Users\\cosim\\projects\\c\\chess\\log.txt";

    void log(std::string message)
    {
        std::ofstream logFile(LOG_FILE, std::ios::out | std::ios::app);
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logFile << std::ctime(&time) << message << std::endl;
        logFile.close();
    }
}

#endif
