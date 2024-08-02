#ifndef UCI
#define UCI

#include <sstream>
#include <string>
#include "bot.hpp"

namespace engine
{
    class UCIEngine
    {
    private:
        Bot bot;

        void respond(std::string message);

        void processPosition(std::istringstream &iss);
        void processGo(std::istringstream &iss);
        void printPosition();

        void runPerft(int depth);

    public:
        UCIEngine() = default;

        void loop();
    };
}

#endif
