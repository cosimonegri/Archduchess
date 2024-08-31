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
        TTtable.clear();

        int depth = 1;
        bool maximize = pos.getTurn() == WHITE;
        uint64_t nodes;
        SearchResult result;
        result.bestMove = Move();

        auto begin = std::chrono::steady_clock::now();
        while (true)
        {
            nodes = search(pos, result, depth, 0, MIN_EVAL, MAX_EVAL, maximize, result.bestMove);
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
        debug("Table:\t" + std::to_string((TTtable.size() * sizeof(TTEntry)) / 1000) + " Kb\n");

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

    uint64_t SearchManager::search(Position &pos, SearchResult &result, int depth,
                                   int ply, Eval alpha, Eval beta, bool maximize, Move bestMove)
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

        TTEntry entry;
        entry.bestMove = Move();
        if (TTtable.contains(pos.getZobristKey()))
        {
            entry = TTtable.at(pos.getZobristKey());
            if (entry.depth >= depth)
            {
                result.eval = entry.eval;
                result.bestMove = entry.bestMove;
                return 1;
            }
        }

        MoveList moveList;
        generateMoves(pos, moveList);

        // set evaluation to the worst possible
        result.eval = maximize ? MIN_EVAL : MAX_EVAL;

        if (moveList.size == 0)
        {
            if (pos.isKingInCheck(maximize ? WHITE : BLACK))
            {
                result.eval += maximize ? ply : -ply;
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
                                        : moveList.moves[i] == entry.bestMove
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
        for (size_t i = 0; i < extMoveList.size; i++)
        {
            pos.makeTurn(extMoveList.moves[i], &state);
            count += search(pos, newResult, depth - 1, ply + 1, alpha, beta, !maximize, Move());
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

        TTEntry newEntry;
        newEntry.bestMove = result.bestMove;
        newEntry.eval = result.eval;
        newEntry.depth = depth;
        TTtable.insert(std::make_pair(pos.getZobristKey(), newEntry));

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
