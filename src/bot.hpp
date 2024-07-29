#ifndef BOT
#define BOT

#include <string>
#include "position.hpp"

namespace engine
{
    class Bot
    {
    private:
        Position pos;

    public:
        Bot();

        Position getPosition();
        void setPosition(std::string fen);
        void makeTurn(std::string move);
    };
}

#endif
