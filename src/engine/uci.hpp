#ifndef UCI
#define UCI

#include <sstream>
#include <string>
#include "bot.hpp"
#include "listeners.hpp"
#include "types.hpp"

namespace engine
{
    class UCIEngine : public MoveListener
    {
    private:
        Bot bot;

        void respond(std::string message);

        void processPosition(std::istringstream &iss);
        void processGo(std::istringstream &iss);
        void printPosition();

        void runPerft(Depth depth);

    public:
        UCIEngine();

        void loop();
        void onReceiveInfo(Depth depth, uint64_t nodes, uint64_t timeMs,
                           float ttOccupancy) override;
        void onMoveChosen(std::string move) override;
    };
}

#endif
