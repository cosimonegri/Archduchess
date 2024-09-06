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

    constexpr int TT_SCORE = 1000000000;
    constexpr int PROM_SCORE = 100000000;
    constexpr int KILLER_SCORE_A = 8000000;
    constexpr int KILLER_SCORE_B = 5000000;

    // victim - attacker
    constexpr int MVV_LVA[7][7] = {
        {0, 0, 0, 0, 0, 0, 0},
        {0, 15000000, 14000000, 13000000, 12000000, 11000000, 10000000}, // P->P, N->P, B->P, R->P, Q->P, K->P
        {0, 25000000, 24000000, 23000000, 22000000, 21000000, 20000000}, // P->N, N->N, B->N, R->N, Q->N, K->N
        {0, 35000000, 34000000, 33000000, 32000000, 31000000, 30000000}, // P->B, N->B, B->B, R->B, Q->B, K->B
        {0, 45000000, 44000000, 43000000, 42000000, 41000000, 40000000}, // P->R, N->R, B->R, R->R, Q->R, K->R
        {0, 55000000, 54000000, 53000000, 52000000, 51000000, 50000000}, // P->P, N->P, B->P, R->P, Q->P, K->P
        {0, 0, 0, 0, 0, 0, 0},
    };

    struct ExtendedMove : Move
    {
        int score;
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
        int history[2][64][64];
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;

        SearchListener *listener;

        bool searching;
        bool cancelled;
        std::mutex searchMutex;

        uint64_t search(Position &pos, SearchResult &result, Depth depth,
                        int ply, Eval alpha, Eval beta, Move bestMove);
        int scoreMove(Position &pos, Move &move, Killers &k);

    public:
        SearchManager();

        void setListener(SearchListener *listener);
        void setSearching();
        void setCancelled();
        bool isCancelled();
        void searchEnded();
        void clear();
        void startSearch(Position &pos);
        Move runIterativeDeepening(Position &pos, Depth maxDepth = MAX_DEPTH,
                                   SearchDiagnostic *sc = NULL);
    };
}

#endif
