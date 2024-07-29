#ifndef UCI
#define UCI

#include <sstream>
#include "bot.hpp"

namespace engine
{
    class UCIEngine
    {
    private:
        Bot bot;

        void processPosition(std::istringstream &iss);
        void processGo(std::istringstream &iss);
        void printPosition();

        void runPerft(int depth);
        static void runPerftTest();

    public:
        UCIEngine() = default;

        void loop();
    };
}

#endif
