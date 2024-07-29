#ifndef MISC_H
#define MISC_H

#include <fstream>
#include <string>
#include <chrono>

namespace engine
{
#ifdef NDEBUG
#define debug(x)
#else
#define debug(x)                     \
    do                               \
    {                                \
        std::cerr << x << std::endl; \
    } while (0)
#endif

    static const std::string LOG_FILE = "C:\\Users\\cosim\\projects\\c\\chess\\log.txt";

    inline void log(std::string message)
    {
        std::ofstream logFile(LOG_FILE, std::ios::out | std::ios::app);
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logFile << std::ctime(&time) << message << std::endl;
        logFile.close();
    }

    inline std::string timeReport(std::chrono::steady_clock::time_point begin, std::chrono::steady_clock::time_point end)
    {
        return "Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()) + " ms";
    }
}

#endif
