#ifndef SEARCH_H
#define SEARCH_H

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
        uint64_t search(Position &pos, SearchResult &result,
                        int depth, Eval alpha, Eval beta, bool maximize);
        int evaluateMove(Position &pos, Move &move);

    public:
        Move getBestMove(Position &pos);
    };
}

#endif
