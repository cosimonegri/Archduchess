#ifndef SEARCH_H
#define SEARCH_H

#include "position.hpp"
#include "move.hpp"

namespace engine
{
    struct SearchResult
    {
        Move bestMove;
        int eval;
    };

    class SearchManager
    {
    private:
        uint64_t search(Position &pos, SearchResult &result, int depth, int alpha, int beta, bool maximize);

    public:
        Move getBestMove(Position &pos);
    };
}

#endif
