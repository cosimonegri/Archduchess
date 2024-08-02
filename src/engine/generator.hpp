#ifndef GENERATOR
#define GENERATOR

#include "position.hpp"
#include "types.hpp"

namespace engine
{
    constexpr int maxMoves = 218;

    struct MoveList
    {
        Move moves[maxMoves];
        size_t size;

        MoveList() : size{0} {};
    };

    void generateMoves(Position &pos, MoveList &moveList);
}

#endif
