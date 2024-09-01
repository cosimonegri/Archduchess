#include "transposition.hpp"

namespace engine
{
    TranspositionTable::TranspositionTable()
    {
        entries = new TTEntry[TT_SIZE];
        clear();
    }

    void TranspositionTable::clear()
    {
        for (size_t i = 0; i < TT_SIZE; i++)
        {
            entries[i].depth = INVALID_DEPTH;
        }
        occupied = 0;
    }

    void TranspositionTable::add(Key key, Depth depth, NodeType type, Move bestMove, Eval eval)
    {
        size_t index = key % TT_SIZE;
        if (entries[index].depth == INVALID_DEPTH)
        {
            occupied++;
        }
        entries[index].key = key;
        entries[index].depth = depth;
        entries[index].type = type;
        entries[index].bestMove = bestMove;
        entries[index].eval = eval;
    }

    TTEntry *TranspositionTable::get(Key key)
    {
        size_t index = key % TT_SIZE;
        return entries[index].isValid() && entries[index].key == key
                   ? entries + index
                   : NULL;
    }

    float TranspositionTable::getOccupancyRate()
    {
        return (float)occupied / (float)TT_SIZE;
    }
}
