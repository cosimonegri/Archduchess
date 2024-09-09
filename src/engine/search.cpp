#include <algorithm>
#include <array>
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
    SearchManager::SearchManager() : TT{TranspositionTable()}, listener{NULL},
                                     searching{false}, cancelled{false} {}

    void SearchManager::setListener(SearchListener *listener)
    {
        this->listener = listener;
    }

    void SearchManager::setSearching()
    {
        searchMutex.lock();
        searching = true;
        searchMutex.unlock();
    }

    void SearchManager::setCancelled()
    {
        searchMutex.lock();
        if (searching)
        {
            cancelled = true;
        }
        searchMutex.unlock();
    }

    bool SearchManager::isCancelled()
    {
        searchMutex.lock();
        bool val = cancelled;
        searchMutex.unlock();
        return val;
    }

    void SearchManager::searchEnded()
    {
        searchMutex.lock();
        searching = false;
        cancelled = false;
        searchMutex.unlock();
    }

    void SearchManager::clear()
    {
        TT.clear();
        for (size_t i = 0; i < std::size(killers); i++)
        {
            killers[i].add(Move());
            killers[i].add(Move());
        }
        for (Color color : {WHITE, BLACK})
            for (Tile from = A1; from <= H8; ++from)
                for (Tile to = A1; to <= H8; ++to)
                    history[color][from][to] = 0;
        cutOffs = 0;
        ttAccesses = 0;
        ttHits = 0;
    }

    void SearchManager::startSearch(Position &pos)
    {
        setSearching();
        Move bestMove = runIterativeDeepening(pos);
        listener->onSearchComplete(bestMove);
        searchEnded();
    }

    Move SearchManager::runIterativeDeepening(Position &pos, Depth maxDepth, SearchDiagnostic *sc)
    {
        // todo maybe don't clear in the future
        clear();

        Depth depth = 1;
        uint64_t nodes = 0;
        SearchResult result;
        result.bestMove = Move();

        auto begin = std::chrono::steady_clock::now();
        while (true)
        {
            nodes += search(pos, result, depth, 0, MIN_EVAL, MAX_EVAL, false);
            auto time = getTimeMs(begin, std::chrono::steady_clock::now());
            if (listener != NULL)
            {
                listener->onSearchInfo(depth, nodes, time, TT.getOccupancyRate());
            }
            if (depth >= maxDepth || isCancelled())
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
                                   int ply, Eval alpha, Eval beta, bool canNull)
    {
        if (isCancelled())
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
            return quiescenceSearch(pos, result, alpha, beta);
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
        generateMoves<ALL>(pos, moveList);

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

        uint64_t count = 0;
        SearchResult newResult;
        RevertState state;

        // todo not during zugzwang
        if (canNull && ply > 0 && depth >= 3 && !pos.isKingInCheck())
        {
            Depth reduction = depth > 6 ? 3 : 2;
            pos.makeNullMove(&state);
            count += search(pos, newResult, depth - 1 - reduction, ply + 1, -beta, -beta + 1, false);
            pos.unmakeNullMove();
            if (-newResult.eval >= beta)
            {
                cutOffs++;
                result.eval = beta;
                return count;
            }
        }

        ExtMoveList extMoveList = ExtMoveList(moveList);
        Move bestMove = ply == 0
                            ? result.bestMove
                        : entry != NULL
                            ? entry->bestMove
                            : Move();
        scoreMoves(pos, extMoveList, bestMove, &killers[ply]);

        result.eval = MIN_EVAL;
        while (extMoveList.size > 0)
        {
            Move move = popMoveHighestScore(extMoveList);
            pos.makeTurn(move, &state);
            count += search(pos, newResult, depth - 1, ply + 1, -beta, -alpha, true);
            pos.unmakeTurn();

            if (isCancelled())
            {
                return count;
            }

            Eval newEval = -newResult.eval;
            if (newEval > result.eval)
            {
                result.eval = newEval;
                result.bestMove = move;
            }

            alpha = std::max(alpha, result.eval);
            if (alpha >= beta)
            {
                cutOffs++;
                if (!move.isCapture())
                {
                    killers[ply].add(move);
                    history[pos.getTurn()][move.getFrom()][move.getTo()] += depth * depth;
                }
                goto exit;
            }
        }

    exit:
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

    uint64_t SearchManager::quiescenceSearch(Position &pos, SearchResult &result, Eval alpha, Eval beta)
    {
        if (isCancelled())
        {
            return 0;
        }

        Eval eval = evaluate(pos);
        if (eval >= beta)
        {
            cutOffs++;
            result.eval = beta;
            return 1;
        }
        alpha = std::max(alpha, eval);

        MoveList moveList;
        generateMoves<CAPTURES>(pos, moveList);

        ExtMoveList extMoveList(moveList);
        scoreMoves(pos, extMoveList, Move());

        SearchResult newResult;
        RevertState state;
        uint64_t count = 0;

        while (extMoveList.size > 0)
        {
            Move move = popMoveHighestScore(extMoveList);
            pos.makeTurn(move, &state);
            count += quiescenceSearch(pos, newResult, -beta, -alpha);
            pos.unmakeTurn();

            if (isCancelled())
            {
                return count;
            }

            Eval newEval = -newResult.eval;
            if (newEval >= beta)
            {
                cutOffs++;
                result.eval = beta;
                return count;
            }
            alpha = std::max(alpha, newEval);
        }

        result.eval = alpha;
        return count;
    }

    void SearchManager::scoreMoves(Position &pos, ExtMoveList &moveList, Move bestMove, Killers *k)
    {
        for (size_t i = 0; i < moveList.size; i++)
        {
            moveList.moves[i].score = moveList.moves[i] == bestMove
                                          ? TT_SCORE
                                          : scoreMove(pos, moveList.moves[i], k);
        }
    }

    int SearchManager::scoreMove(Position &pos, Move &move, Killers *k)
    {
        int score = 0;
        Piece piece = pos.getPiece(move.getFrom());
        Piece captured = pos.getPiece(move.getTo());
        if (move.isPromotion())
        {
            score += PROM_SCORE;
        }
        score += MVV_LVA[typeOf(captured)][typeOf(piece)];
        if (captured == NULL_PIECE)
        {
            if (k != NULL && k->matchA(move))
            {
                score += KILLER_SCORE_A;
            }
            else if (k != NULL && k->matchB(move))
            {
                score += KILLER_SCORE_B;
            }
            score += history[pos.getTurn()][move.getFrom()][move.getTo()];
        }
        return score;
    }

    Move SearchManager::popMoveHighestScore(ExtMoveList &moveList)
    {
        assert(moveList.size > 0);
        size_t moveIndex = 0;
        for (size_t j = 1; j < moveList.size; j++)
        {
            if (moveList.moves[j].score > moveList.moves[moveIndex].score)
                moveIndex = j;
        }
        Move move = moveList.moves[moveIndex];
        moveList.moves[moveIndex] = moveList.moves[moveList.size - 1];
        moveList.size--;
        return move;
    }
}
