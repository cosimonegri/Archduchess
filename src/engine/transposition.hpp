#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "evaluation.hpp"
#include "zobrist.hpp"
#include "move.hpp"

namespace engine
{
    constexpr size_t TT_SIZE = 1 << 22;

    constexpr Depth INVALID_DEPTH = -1;

    struct TTEntry
    {
        Key key;
        Depth depth;
        Move bestMove;
        Eval eval;

        bool isValid() const
        {
            return depth != INVALID_DEPTH;
        }
    };

    class TranspositionTable
    {
    private:
        TTEntry *entries;
        size_t occupied;

    public:
        TranspositionTable();

        void clear();
        void add(Key key, Depth depth, Move bestMove, Eval eval);
        TTEntry *get(Key key);
        float getOccupancyRate();
    };
}

#endif
