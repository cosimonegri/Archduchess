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

    // victim - attacker
    constexpr int MVV_LVA[7][7] = {
        {0, 0, 0, 0, 0, 0, 0},
        {0, 15, 14, 13, 12, 11, 10}, // P->P, N->P, B->P, R->P, Q->P, K->P
        {0, 25, 24, 23, 22, 21, 20}, // P->N, N->N, B->N, R->N, Q->N, K->N
        {0, 35, 34, 33, 32, 31, 30}, // P->B, N->B, B->B, R->B, Q->B, K->B
        {0, 45, 44, 43, 42, 41, 40}, // P->R, N->R, B->R, R->R, Q->R, K->R
        {0, 55, 54, 53, 52, 51, 50}, // P->P, N->P, B->P, R->P, Q->P, K->P
        {0, 0, 0, 0, 0, 0, 0},
    };

    constexpr int KILLER_SCORE_A = 8;
    constexpr int KILLER_SCORE_B = 5;

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

    struct Killers
    {
        Move moveA;
        Move moveB;

        void add(Move move)
        {
            if (move != moveA)
            {
                moveB = moveA;
                moveA = move;
            }
        }

        bool matchA(Move move)
        {
            return move == moveA;
        }

        bool matchB(Move move)
        {
            return move == moveB;
        }
    };

    class SearchManager
    {
    private:
        TranspositionTable TT;
        Killers killers[MAX_DEPTH + 1];
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;

        SearchListener *listener;

        bool cancel;
        std::mutex cancelMtx;

        uint64_t search(Position &pos, SearchResult &result, Depth depth,
                        int ply, Eval alpha, Eval beta, Move bestMove);
        int scoreMove(Position &pos, Move &move, Killers &k);

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
