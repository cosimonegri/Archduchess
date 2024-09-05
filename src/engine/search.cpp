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
    SearchManager::SearchManager() : TT{TranspositionTable()}, listener{NULL}, cancel{false} {}

    void SearchManager::setListener(SearchListener *listener)
    {
        this->listener = listener;
    }

    void SearchManager::setCancel()
    {
        cancelMtx.lock();
        cancel = true;
        cancelMtx.unlock();
    }

    void SearchManager::clearCancel()
    {
        cancelMtx.lock();
        cancel = false;
        cancelMtx.unlock();
    }

    bool SearchManager::getCancel()
    {
        cancelMtx.lock();
        bool canc = cancel;
        cancelMtx.unlock();
        return canc;
    }

    void SearchManager::clear()
    {
        TT.clear();
    }

    void SearchManager::startSearch(Position &pos)
    {
        Move bestMove = runIterativeDeepening(pos);
        listener->onSearchComplete(bestMove);
        clearCancel();
    }

    Move SearchManager::runIterativeDeepening(Position &pos, Depth maxDepth, SearchDiagnostic *sc)
    {
        // todo maybe don't clear in the future
        TT.clear();
        cutOffs = 0;
        ttAccesses = 0;
        ttHits = 0;

        Depth depth = 1;
        uint64_t nodes = 0;
        SearchResult result;
        result.bestMove = Move();

        auto begin = std::chrono::steady_clock::now();
        while (true)
        {
            nodes += search(pos, result, depth, 0, MIN_EVAL, MAX_EVAL, result.bestMove);
            auto time = getTimeMs(begin, std::chrono::steady_clock::now());
            if (listener != NULL)
            {
                listener->onSearchInfo(depth, nodes, time, TT.getOccupancyRate());
            }
            if (depth >= maxDepth || getCancel())
            {
                break;
            }
            depth += 1;
        }
        int64_t totalTime = getTimeMs(begin, std::chrono::steady_clock::now());

        if (sc != NULL)
        {
            sc->depth = depth;
            sc->nodes = nodes;
            sc->timeMs = totalTime;
            sc->cutOffs = cutOffs;
            sc->ttAccesses = ttAccesses;
            sc->ttHits = ttHits;
            sc->ttOccupancy = TT.getOccupancyRate();
        }

        debug("Depth:\t" + std::to_string(depth));
        debug("Nodes:\t" + std::to_string(nodes));
        debug("Time:\t" + std::to_string(totalTime) + " ms");
        debug("NPS:\t" + std::to_string(nodes / totalTime) + "k");
        debug("TT:\t" + std::to_string(TT.getOccupancyRate() * 100) + "% of " +
              std::to_string(TT_SIZE * sizeof(TTEntry) / 1048576) + " MB\n");

        return result.bestMove;
    }

    uint64_t SearchManager::search(Position &pos, SearchResult &result, Depth depth,
                                   int ply, Eval alpha, Eval beta, Move bestMove)
    {
        if (getCancel())
        {
            return 0;
        }

        if (pos.getHalfMove() >= 100 || pos.isRepeated())
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
        ttAccesses++;
        if (entry != NULL && entry->depth >= depth)
        {
            ttHits++;
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
            std::sort(extMoveList.moves, extMoveList.moves + extMoveList.size, [](const ExtendedMove &a, const ExtendedMove &b)
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

            if (getCancel())
            {
                return count;
            }

            if (-newResult.eval > result.eval)
            {
                result.eval = -newResult.eval;
                result.bestMove = extMoveList.moves[i];
            }

            alpha = std::max(alpha, result.eval);
            if (alpha >= beta)
            {
                cutOffs++;
                break;
            }
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
