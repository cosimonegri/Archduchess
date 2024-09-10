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
                                     thinkInfo{NULL} {}

    void SearchManager::setListener(SearchListener *listener)
    {
        this->listener = listener;
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

        moveToMake = Move();
        nodes = 0;
        qNodes = 0;
        cutOffs = 0;
        ttAccesses = 0;
        ttHits = 0;
    }

    void SearchManager::startSearch(Position &pos, ThinkInfo *info)
    {
        uint64_t thinkTime = calcThinkTimeMs(*info, pos.getTurn());
        thinkInfo = info;
        startTime = std::chrono::steady_clock::now();
        endTime = startTime + std::chrono::milliseconds(thinkTime);
        Move bestMove = runIterativeDeepening(pos);
        thinkInfo = NULL;
        listener->onSearchComplete(bestMove);
    }

    Move SearchManager::runIterativeDeepening(Position &pos, Depth maxDepth, SearchDiagnostic *sc)
    {
        // todo maybe don't clear in the future
        clear();

        Depth depth = 1;
        auto begin = std::chrono::steady_clock::now();
        while (true)
        {
            search(pos, depth, 0, MIN_EVAL, MAX_EVAL, false);
            auto time = getTimeMs(begin, std::chrono::steady_clock::now());
            if (listener != NULL)
            {
                listener->onSearchInfo(depth, nodes, time, TT.getOccupancyRate());
            }
            if (depth >= maxDepth || shouldStop(thinkInfo, depth, nodes, endTime))
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
            sc->qNodes = qNodes;
            sc->timeMs = totalTime;
            sc->cutOffs = cutOffs;
            sc->ttAccesses = ttAccesses;
            sc->ttHits = ttHits;
            sc->ttOccupancy = TT.getOccupancyRate();
        }

        return moveToMake;
    }

    Eval SearchManager::search(Position &pos, Depth depth, int ply,
                               Eval alpha, Eval beta, bool canNull)
    {
        if (shouldStop(thinkInfo, 0, nodes, endTime))
        {
            return 0;
        }

        if (pos.getHalfMove() >= 100 || pos.isRepeated())
        {
            nodes++;
            return 0;
        }

        if (depth <= 0)
        {
            return quiescenceSearch(pos, alpha, beta);
        }

        Eval originalAlpha = alpha;
        TTEntry *entry = TT.get(pos.getZobristKey());
        ttAccesses++;
        if (ply > 0 && entry != NULL && entry->depth >= depth)
        {
            ttHits++;
            if (entry->type == EXACT)
            {
                nodes++;
                return entry->eval;
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
                nodes++;
                return entry->eval;
            }
        }

        MoveList moveList;
        generateMoves<ALL>(pos, moveList);

        if (moveList.size == 0)
        {
            nodes++;
            if (pos.isKingInCheck())
            {
                return MIN_EVAL + ply;
            }
            else
            {
                return 0;
            }
        }

        Eval eval;
        RevertState state;

        // todo not during zugzwang
        if (canNull && ply > 0 && depth >= 3 && !pos.isKingInCheck())
        {
            Depth reduction = depth > 6 ? 3 : 2;
            pos.makeNullMove(&state);
            eval = -search(pos, depth - 1 - reduction, ply + 1, -beta, -beta + 1, false);
            pos.unmakeNullMove();
            if (eval >= beta)
            {
                cutOffs++;
                return beta;
            }
        }

        ExtMoveList extMoveList = ExtMoveList(moveList);
        Move hashMove = ply == 0
                            ? moveToMake
                        : entry != NULL
                            ? entry->hashMove
                            : Move();
        scoreMoves(pos, extMoveList, hashMove, &killers[ply]);

        Eval bestEval = MIN_EVAL;
        Move bestMove = Move();

        while (extMoveList.size > 0)
        {
            Move move = popMoveHighestScore(extMoveList);
            pos.makeTurn(move, &state);
            eval = -search(pos, depth - 1, ply + 1, -beta, -alpha, true);
            pos.unmakeTurn();

            if (shouldStop(thinkInfo, 0, nodes, endTime))
            {
                return 0;
            }

            if (eval > bestEval)
            {
                bestEval = eval;
                bestMove = move;
            }

            alpha = std::max(alpha, eval);
            if (alpha >= beta)
            {
                cutOffs++;
                if (!move.isCapture())
                {
                    killers[ply].add(move);
                    history[pos.getTurn()][move.getFrom()][move.getTo()] += depth * depth;
                }
                break;
            }
        }

        NodeType type = EXACT;
        if (bestEval >= beta)
        {
            type = LOWER_BOUND;
        }
        else if (bestEval <= originalAlpha)
        {
            type = UPPER_BOUND;
        }
        TT.add(pos.getZobristKey(), depth, type, bestMove, bestEval);

        if (ply == 0)
        {
            moveToMake = bestMove;
        }
        return bestEval;
    }

    Eval SearchManager::quiescenceSearch(Position &pos, Eval alpha, Eval beta)
    {
        if (shouldStop(thinkInfo, 0, nodes, endTime))
        {
            return 0;
        }

        Eval standPat = evaluate(pos);
        if (standPat >= beta)
        {
            nodes++;
            qNodes++;
            cutOffs++;
            return beta;
        }
        alpha = std::max(alpha, standPat);

        MoveList moveList;
        generateMoves<CAPTURES>(pos, moveList);

        ExtMoveList extMoveList(moveList);
        scoreMoves(pos, extMoveList, Move());

        Eval eval;
        RevertState state;
        while (extMoveList.size > 0)
        {
            Move move = popMoveHighestScore(extMoveList);

            // todo don't do delta pruning in endgame
            if (!move.isPromotion() &&
                standPat + getPieceEval(typeOf(pos.getPiece(move.getTo()))) + 200 < alpha)
            {
                continue;
            }

            pos.makeTurn(move, &state);
            eval = -quiescenceSearch(pos, -beta, -alpha);
            pos.unmakeTurn();

            if (shouldStop(thinkInfo, 0, nodes, endTime))
            {
                return 0;
            }

            if (eval >= beta)
            {
                cutOffs++;
                return beta;
            }
            alpha = std::max(alpha, eval);
        }

        return alpha;
    }

    void SearchManager::scoreMoves(Position &pos, ExtMoveList &moveList, Move hashMove, Killers *k)
    {
        for (size_t i = 0; i < moveList.size; i++)
        {
            moveList.moves[i].score = moveList.moves[i] == hashMove
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
