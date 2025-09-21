#ifndef SEARCH_H
#define SEARCH_H

#include <mutex>
#include "transposition.hpp"
#include "evaluation.hpp"
#include "position.hpp"
#include "generator.hpp"
#include "time.hpp"
#include "listeners.hpp"
#include "move.hpp"

namespace engine
{
    constexpr Depth MAX_DEPTH = 100;

    constexpr int BASE_SCORE_MULTIPLIER = 1000000;

    constexpr int TT_SCORE = 1000 * BASE_SCORE_MULTIPLIER;
    constexpr int PROM_SCORE = 100 * BASE_SCORE_MULTIPLIER;
    constexpr int KILLER_SCORE_A = 8 * BASE_SCORE_MULTIPLIER;
    constexpr int KILLER_SCORE_B = 5 * BASE_SCORE_MULTIPLIER;

    // victim -> attacker
    constexpr int MVV_LVA[7][7] = {
        {0, 0, 0, 0, 0, 0, 0},       // no victim, attacker None, P, N, B, R, Q, K
        {0, 15, 14, 13, 12, 11, 10}, // victim P, attacker None, P, N, B, R, Q, K
        {0, 25, 24, 23, 22, 21, 20}, // victim N, attacker None, P, N, B, R, Q, K
        {0, 35, 34, 33, 32, 31, 30}, // victim B, attacker None, P, N, B, R, Q, K
        {0, 45, 44, 43, 42, 41, 40}, // victim R, attacker None, P, N, B, R, Q, K
        {0, 55, 54, 53, 52, 51, 50}, // victim Q, attacker None, P, N, B, R, Q, K
        {0, 0, 0, 0, 0, 0, 0},       // victim K, attacker None, P, N, B, R, Q, K
    };
    constexpr int MVV_LVA_SCORE_MULTIPLIER = BASE_SCORE_MULTIPLIER;

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

    struct SearchDiagnostic
    {
        Depth depth;
        uint64_t nodes;
        uint64_t qNodes;
        uint64_t timeMs;
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;
        float ttOccupancy;
    };

    struct Killers
    {
    private:
        Move moveA;
        Move moveB;

    public:
        void add(Move move)
        {
            if (move != moveA)
            {
                moveB = moveA;
                moveA = move;
            }
        }

        void clear()
        {
            moveA = Move();
            moveB = Move();
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

        Move moveToMake;
        uint64_t nodes;
        uint64_t qNodes;
        uint64_t cutOffs;
        uint64_t ttAccesses;
        uint64_t ttHits;

        ThinkInfo *thinkInfo;
        std::chrono::_V2::steady_clock::time_point startTime;
        std::chrono::_V2::steady_clock::time_point endTime;

        SearchListener *listener;

        Eval search(Position &pos, Depth depth, int ply, Eval alpha, Eval beta, bool canNull);
        Eval quiescenceSearch(Position &pos, Eval alpha, Eval beta);
        void scoreMoves(Position &pos, ExtMoveList &moveList, Move hashMove, Killers *k = NULL);
        int scoreMove(Position &pos, Move &move, Killers *k = NULL);
        Move popMoveHighestScore(ExtMoveList &moveList);

    public:
        SearchManager();

        void setListener(SearchListener *listener);
        void clear();
        void startSearch(Position &pos, ThinkInfo *info);
        Move runIterativeDeepening(Position &pos, Depth maxDepth = MAX_DEPTH,
                                   SearchDiagnostic *sc = NULL);
    };
}

#endif
