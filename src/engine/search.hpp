#ifndef SEARCH_H
#define SEARCH_H

#include <mutex>
#include "transposition.hpp"
#include "evaluation.hpp"
#include "position.hpp"
#include "generator.hpp"
#include "listeners.hpp"
#include "move.hpp"

namespace engine
{
    constexpr Depth MAX_DEPTH = 100;

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

    struct SearchDiagnostic
    {
        Depth depth;
        uint64_t nodes;
        uint64_t timeMs;
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;
        float ttOccupancy;
    };

    class SearchManager
    {
    private:
        TranspositionTable TT;
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;

        SearchListener *listener;

        bool cancel;
        std::mutex cancelMtx;

        uint64_t search(Position &pos, SearchResult &result, Depth depth,
                        int ply, Eval alpha, Eval beta, Move bestMove);
        int evaluateMove(Position &pos, Move &move);

    public:
        SearchManager();

        void setListener(SearchListener *listener);
        void setCancel();
        void clearCancel();
        bool getCancel();
        void clear();
        void startSearch(Position &pos);
        Move runIterativeDeepening(Position &pos, Depth maxDepth = MAX_DEPTH,
                                   SearchDiagnostic *sc = NULL);
    };
}

#endif
