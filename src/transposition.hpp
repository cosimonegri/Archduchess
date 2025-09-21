#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "position.hpp"
#include "evaluation.hpp"
#include "move.hpp"

namespace engine
{
    enum NodeType : uint8_t
    {
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND,
    };

    constexpr size_t TT_SIZE = 1 << 22;
    constexpr Depth INVALID_DEPTH = -1;

    struct TTEntry
    {
        Key key;
        Depth depth;
        NodeType type;
        Move hashMove;
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
        ~TranspositionTable();

        void clear();
        void add(const Position &pos, Depth depth, NodeType type, Move hashMove, Eval eval);
        TTEntry *get(const Position &pos);
        float getOccupancyRate();
    };
}

#endif
