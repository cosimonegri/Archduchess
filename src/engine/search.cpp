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
    SearchManager::SearchManager() : TT{TranspositionTable()} {}

    Move SearchManager::getBestMove(Position &pos)
    {
        Depth depth = 1;
        uint64_t nodes;
        SearchResult result;
        result.bestMove = Move();

        auto begin = std::chrono::steady_clock::now();
        while (true)
        {
            nodes = search(pos, result, depth, 0, MIN_EVAL, MAX_EVAL, result.bestMove);
            auto current = std::chrono::steady_clock::now();
            if (getTimeMs(begin, current) >= 100)
            {
                break;
            }
            depth += 1;
        }
        auto end = std::chrono::steady_clock::now();
        int64_t elapsedTime = getTimeMs(begin, end);

        debug("Depth:\t" + std::to_string(depth));
        debug("Nodes:\t" + std::to_string(nodes));
        debug("Time:\t" + std::to_string(elapsedTime) + " ms");
        debug("NPS:\t" + std::to_string(nodes / elapsedTime) + "k");
        debug("TT:\t" + std::to_string(TT.getOccupancyRate() * 100) + "% of " +
              std::to_string(TT_SIZE * sizeof(TTEntry) / 1048576) + " MB\n");

        if (result.bestMove.raw() == 0)
        {
            MoveList moveList;
            generateMoves(pos, moveList);
            if (moveList.size != 0)
            {
                result.bestMove = moveList.moves[0];
            }
        }
        return result.bestMove;
    }

    uint64_t SearchManager::search(Position &pos, SearchResult &result, Depth depth,
                                   int ply, Eval alpha, Eval beta, Move bestMove)
    {
        if (pos.isRepeated())
        {
            result.eval = 0;
            return 1;
        }

        if (depth <= 0)
        {
            result.eval = evaluate(pos);
            return 1;
        }

        Eval originalAlpha = alpha;
        TTEntry *entry = TT.get(pos.getZobristKey());
        if (entry != NULL && entry->depth >= depth)
        {
            if (entry->type == EXACT)
            {
                result.eval = entry->eval;
                result.bestMove = entry->bestMove;
                return 1;
            }
            else if (entry->type == LOWER_BOUND)
            {
                alpha = std::max(alpha, entry->eval);
            }
            else if (entry->type == UPPER_BOUND)
            {
                beta = std::min(beta, entry->eval);
            }

            if (alpha >= beta)
            {
                result.eval = entry->eval;
                result.bestMove = entry->bestMove;
                return 1;
            }
        }

        MoveList moveList;
        generateMoves(pos, moveList);

        if (moveList.size == 0)
        {
            if (pos.isKingInCheck())
            {
                result.eval = MIN_EVAL + ply;
            }
            else
            {
                result.eval = 0;
            }
            return 1;
        }

        ExtMoveList extMoveList = ExtMoveList(moveList);
        for (size_t i = 0; i < extMoveList.size; i++)
        {
            // maybe no need for bestMove because hashMove is
            extMoveList.moves[i].eval = moveList.moves[i] == bestMove
                                            ? MAX_EVAL
                                        : entry != NULL && moveList.moves[i] == entry->bestMove
                                            ? MAX_EVAL - 10
                                            : evaluateMove(pos, moveList.moves[i]);
        }
        if (extMoveList.size > 1)
        {
            // todo maybe there should not ba a -1
            std::sort(extMoveList.moves, extMoveList.moves + extMoveList.size - 1, [](const ExtendedMove &a, const ExtendedMove &b)
                      { return a.eval > b.eval; });
        }

        SearchResult newResult;
        RevertState state;
        uint64_t count = 0;
        result.eval = MIN_EVAL;

        for (size_t i = 0; i < extMoveList.size; i++)
        {
            pos.makeTurn(extMoveList.moves[i], &state);
            count += search(pos, newResult, depth - 1, ply + 1, -beta, -alpha, Move());
            pos.unmakeTurn();

            if (-newResult.eval > result.eval)
            {
                result.eval = -newResult.eval;
                result.bestMove = extMoveList.moves[i];
            }

            alpha = std::max(alpha, result.eval);
            if (alpha >= beta)
                break;
        }

        NodeType type = EXACT;
        if (result.eval >= beta)
        {
            type = LOWER_BOUND;
        }
        else if (result.eval <= originalAlpha)
        {
            type = UPPER_BOUND;
        }
        TT.add(pos.getZobristKey(), depth, type, result.bestMove, result.eval);

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
            Piece piece = pos.getPiece(move.getFrom());
            eval += 1000 + getPieceEval(typeOf(captured)) - getPieceEval(typeOf(piece));
        }
        return eval;
    }
}
