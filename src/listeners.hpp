#ifndef LISTENRS_H
#define LISTENRS_H

#include <string>
#include "move.hpp"
#include "types.hpp"

namespace engine
{
    class MoveListener
    {
    public:
        virtual void onReceiveInfo(Depth depth, uint64_t nodes, uint64_t timeMs, float ttOccupancy);
        virtual void onMoveChosen(std::string move);
    };

    class SearchListener
    {
    public:
        virtual void onSearchInfo(Depth depth, uint64_t nodes, uint64_t timeMs, float ttOccupancy);
        virtual void onSearchComplete(Move move);
    };
}

#endif
