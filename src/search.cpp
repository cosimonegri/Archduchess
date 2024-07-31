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

        debug("Nodes:\t" + std::to_string(nodes));
        debug("Time:\t" + std::to_string(elapsedTime) + " ms");
        debug("NPS:\t" + std::to_string(nodes / elapsedTime) + "k\n");

        return result.bestMove;
    }

    uint64_t SearchManager::search(Position &pos, SearchResult &result,
                                   int depth, Eval alpha, Eval beta, bool maximize)
    {
        if (depth <= 0)
        {
            result.eval = evaluate(pos);
            return 1;
        }

        SearchResult newResult;
        RevertState state;
        MoveList moveList;
        generateMoves(pos, moveList);

        ExtMoveList extMoveList = ExtMoveList(moveList);
        for (size_t i = 0; i < extMoveList.size; i++)
        {
            extMoveList.moves[i].eval = evaluateMove(pos, moveList.moves[i]);
        }
        if (extMoveList.size > 1)
        {
            std::sort(extMoveList.moves, extMoveList.moves + extMoveList.size - 1, [](const ExtendedMove &a, const ExtendedMove &b)
                      { return a.eval > b.eval; });
        }

        // set evaluation to the worst possible
        result.eval = maximize ? MIN_EVAL : MAX_EVAL;

        uint64_t count = 0;
        for (size_t i = 0; i < extMoveList.size; i++)
        {
            pos.makeTurn(extMoveList.moves[i], &state);
            count += search(pos, newResult, depth - 1, alpha, beta, !maximize);
            pos.unmakeTurn();

            if ((maximize && newResult.eval > result.eval) ||
                (!maximize && newResult.eval < result.eval))
            {
                result.eval = newResult.eval;
                result.bestMove = extMoveList.moves[i];
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

    int SearchManager::evaluateMove(Position &pos, Move &move)
    {
        int eval = 0;
        Piece captured = pos.getPiece(move.getTo());
        if (move.isPromotion())
        {
            eval += 2000;
        }
        if (captured != NULL_PIECE)
        {
            eval += 1000 + getPieceEval(captured) - getPieceEval(pos.getPiece(move.getFrom()));
        }
        return eval;
    }
}
