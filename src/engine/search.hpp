#ifndef SEARCH_H
#define SEARCH_H

#include <unordered_map>
#include "transposition.hpp"
#include "evaluation.hpp"
#include "position.hpp"
#include "generator.hpp"
#include "move.hpp"

namespace engine
{
    struct ExtendedMove : Move
    {
        int eval;
        void operator=(Move m) { data = m.raw(); }
    };

    struct ExtMoveList
    {
        ExtendedMove moves[maxMoves];
        size_t size;

        ExtMoveList(MoveList moveList) : size{moveList.size}
        {
            for (size_t i = 0; i < moveList.size; i++)
                moves[i] = moveList.moves[i];
        };
    };

    struct SearchResult
    {
        Move bestMove;
        Eval eval;
    };

    class SearchManager
    {
    private:
        TranspositionTable TT;

        uint64_t search(Position &pos, SearchResult &result, Depth depth,
                        int ply, Eval alpha, Eval beta, Move bestMove);
        int evaluateMove(Position &pos, Move &move);

    public:
        SearchManager();

        Move getBestMove(Position &pos);
    };
}

#endif
