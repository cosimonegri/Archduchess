#include <algorithm>
#include <limits>
#include <chrono>
#include <cassert>
#include "search.hpp"
#include "evaluation.hpp"
#include "generator.hpp"
#include "move.hpp"
#include "types.hpp"
#include "misc.hpp"

namespace engine
{
    Move SearchManager::getBestMove(Position &pos)
    {
        SearchResult result;
        auto begin = std::chrono::steady_clock::now();
        uint64_t nodes = search(pos, result, 6, MIN_EVAL, MAX_EVAL, pos.getTurn() == WHITE);
        auto end = std::chrono::steady_clock::now();
        int64_t elapsedTime = getTimeMs(begin, end);

        debug("Nodes evaluated: " + std::to_string(nodes));
        debug("Time: " + std::to_string(elapsedTime) + "ms");
        debug("NPS: " + std::to_string(nodes / elapsedTime) + "k\n");

        return result.bestMove;
    }

    uint64_t SearchManager::search(Position &pos, SearchResult &result, int depth, int alpha, int beta, bool maximize)
    {
        assert(depth >= 0);
        if (depth == 0)
        {
            result.eval = evaluate(pos);
            return 1;
        }

        SearchResult newResult;
        RevertState state;
        MoveList moveList, sortedList;
        generateMoves(pos, moveList);

        for (size_t i = 0; i < moveList.size; i++)
            if (moveList.moves[i].isPromotion())
                sortedList.moves[sortedList.size++] = moveList.moves[i];

        for (size_t i = 0; i < moveList.size; i++)
            if (moveList.moves[i].isCapture() && !moveList.moves[i].isPromotion())
                sortedList.moves[sortedList.size++] = moveList.moves[i];

        for (size_t i = 0; i < moveList.size; i++)
            if (!moveList.moves[i].isCapture() && !moveList.moves[i].isPromotion())
                sortedList.moves[sortedList.size++] = moveList.moves[i];

        // set evaluation to the worst possible
        result.eval = maximize ? MIN_EVAL : MAX_EVAL;

        uint64_t count = 0;
        for (size_t i = 0; i < sortedList.size; i++)
        {
            pos.makeTurn(sortedList.moves[i], &state);
            count += search(pos, newResult, depth - 1, alpha, beta, !maximize);
            pos.unmakeTurn();

            if ((maximize && newResult.eval > result.eval) ||
                (!maximize && newResult.eval < result.eval))
            {
                result.eval = newResult.eval;
                result.bestMove = sortedList.moves[i];
            }

            if (alpha >= beta)
                break;

            if (maximize)
                alpha = std::max(alpha, result.eval);
            else
                beta = std::min(beta, result.eval);
        }
        return count;
    }
}
