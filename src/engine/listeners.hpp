#ifndef LISTENRS_H
#define LISTENRS_H

#include <string>
#include "move.hpp"

namespace engine
{
    class MoveListener
    {
    public:
        virtual void onMoveChosen(std::string move);
    };

    class SearchListener
    {
    public:
        virtual void onSearchComplete(Move move);
    };
}

#endif
