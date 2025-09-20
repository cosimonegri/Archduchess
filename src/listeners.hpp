#ifndef LISTENERS_H
#define LISTENERS_H

#include <string>
#include "move.hpp"
#include "types.hpp"

namespace engine
{
    class MoveListener
    {
    public:
        virtual void onReceiveInfo(Depth depth, uint64_t nodes, uint64_t timeMs, float ttOccupancy) = 0;
        virtual void onMoveChosen(std::string move) = 0;
    };

    class SearchListener
    {
    public:
        virtual void onSearchInfo(Depth depth, uint64_t nodes, uint64_t timeMs, float ttOccupancy) = 0;
        virtual void onSearchComplete(Move move) = 0;
    };
}

#endif
