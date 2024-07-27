#ifndef GENERATOR
#define GENERATOR

#include "types.hpp"
#include "position.hpp"

namespace engine
{
    constexpr int maxMoves = 218;

    struct MoveList
    {
        Move moves[maxMoves];
        size_t size;

        MoveList() : size{0} {};

        bool contains(Move move);
        void clear();
    };

    void generateMoves(Position &pos, MoveList &moveList);
}

#endif
