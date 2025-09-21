#include "transposition.hpp"

namespace engine
{
    TranspositionTable::TranspositionTable()
    {
        entries = new TTEntry[TT_SIZE];
        clear();
    }

    TranspositionTable::~TranspositionTable()
    {
        delete[] entries;
    }

    /**
     * Clears the transposition table by marking all entries as invalid.
     */
    void TranspositionTable::clear()
    {
        for (size_t i = 0; i < TT_SIZE; i++)
        {
            entries[i].depth = INVALID_DEPTH;
        }
        occupied = 0;
    }

    /**
     * Adds an entry to the transposition table.
     * If an entry already exists at the computed index, it will be overwritten.
     *
     * @param pos The position evaluated.
     * @param depth The search depth at which the evaluation was made.
     * @param type The type of the evaluation (EXACT, LOWER_BOUND, UPPER_BOUND).
     * @param hashMove The best move found from this position.
     * @param eval The evaluation score of the position.
     */
    void TranspositionTable::add(const Position &pos, Depth depth, NodeType type, Move hashMove, Eval eval)
    {
        Key key = pos.getZobristKey();
        size_t index = key % TT_SIZE;
        if (entries[index].depth == INVALID_DEPTH)
        {
            occupied++;
        }
        entries[index].key = key;
        entries[index].depth = depth;
        entries[index].type = type;
        entries[index].hashMove = hashMove;
        entries[index].eval = eval;
    }

    /**
     * Retrieves an entry from the transposition table corresponding to the given position.
     *
     * @param pos The position to look up.
     * @return A pointer to the TTEntry if found and valid, otherwise NULL.
     */
    TTEntry *TranspositionTable::get(const Position &pos)
    {
        Key key = pos.getZobristKey();
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
