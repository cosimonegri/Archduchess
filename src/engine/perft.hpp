#ifndef PERFT
#define PERFT

#include <iostream>
#include <cstdint>
#include "position.hpp"
#include "generator.hpp"
#include "types.hpp"

namespace engine
{
    uint64_t perft(Position &pos, Depth depth, bool root = true)
    {
        if (depth <= 0)
        {
            return 1;
        }

        RevertState state;
        MoveList moveList = MoveList();
        generateMoves<ALL>(pos, moveList);

        if (depth == 1 && !root)
            return moveList.size;

        uint64_t count = 0;
        for (size_t i = 0; i < moveList.size; i++)
        {
            pos.makeTurn(moveList.moves[i], &state);
            uint64_t newCount = perft(pos, depth - 1, false);
            pos.unmakeTurn();

            count += newCount;
            if (root)
                std::cout << moveList.moves[i] << ": " << newCount << std::endl;
        }

        return count;
    }
}

#endif
