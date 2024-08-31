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
        void onMoveChosen(std::string move) override;
    };
}

#endif
