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

    /**
     * Clears the search state, including the transposition table, killer moves,
     * history heuristic and variables used for search diagnostics.
     */
    void SearchManager::clear()
    {
        TT.clear();

        for (size_t i = 0; i < std::size(killers); i++)
            killers[i].clear();

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
        Eval eval = 0;
        Eval alpha, beta;
        auto begin = std::chrono::steady_clock::now();

        while (true)
        {
            // perform a full window search for low depths
            if (depth <= 3)
            {
                eval = search(pos, depth, 0, MIN_EVAL, MAX_EVAL, false);
            }
            // use aspiration windows for higher depths
            else
            {
                alpha = std::max(Eval(eval - WINDOW_DELTA_1), MIN_EVAL);
                beta = std::min(Eval(eval + WINDOW_DELTA_1), MAX_EVAL);
                eval = search(pos, depth, 0, alpha, beta, false);

                // if the result is outside the window, re-search with a wider window
                // changing only the bound that failed
                if (eval <= alpha)
                {
                    alpha = std::max(Eval(eval - WINDOW_DELTA_2), MIN_EVAL);
                    eval = search(pos, depth, 0, alpha, beta, false);
                    // full window search if it fails again
                    if (eval <= alpha)
                    {
                        eval = search(pos, depth, 0, MIN_EVAL, beta, false);
                    }
                }
                if (eval >= beta)
                {
                    beta = std::min(Eval(eval + WINDOW_DELTA_2), MAX_EVAL);
                    eval = search(pos, depth, 0, alpha, beta, false);
                    // full window search if it fails again
                    if (eval >= beta)
                    {
                        eval = search(pos, depth, 0, alpha, MAX_EVAL, false);
                    }
                }
            }

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

    /**
     * Negamax search with alpha-beta pruning and many optimization techniques.
     *
     * @param pos The current position
     * @param depth Half-moves from the leaf nodes of the standard search
     * @param ply Half-moves since the root position
     * @param alpha Represents the best score that the side to move can currently guarantee
     * @param beta Represents the best score that the opponent can currently guarantee
     * @param canNull Whether null move is allowed
     * @return Evaluation of the position, positive means advantage for the side to move
     */
    Eval SearchManager::search(Position &pos, Depth depth, int ply,
                               Eval alpha, Eval beta, bool canNull)
    {
        if (shouldStop(thinkInfo, 0, nodes, endTime))
        {
            return 0;
        }

        // draw by 50-move rule or repetition
        if (pos.getHalfMove() >= 100 || pos.isRepeated())
        {
            nodes++;
            return 0;
        }

        // standard search ends, go to quiescence search
        if (depth <= 0)
        {
            return quiescenceSearch(pos, alpha, beta);
        }

        Eval originalAlpha = alpha;
        TTEntry *entry = TT.get(pos);
        ttAccesses++;

        // Use TT entry only if the position has been evaluated at a depth
        // equal or bigger than the current search depth.
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
                // side to move is in checkmate
                // ply is used to prefer faster/slower checkmates
                return MIN_EVAL + ply;
            }
            else
            {
                // stalemate
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

        // standard negamax search with alpha-beta pruning
        while (extMoveList.size > 0)
        {
            // perform move ordering
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

            // beta-cutoff
            if (alpha >= beta)
            {
                cutOffs++;
                // store killer move and update history heuristic for quite moves
                if (!move.isCapture() && !move.isPromotion())
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
            // If a beta-cutoff has occurred, the evaluation is a lower bound.
            // (the exact score might be higher)
            type = LOWER_BOUND;
        }
        else if (bestEval <= originalAlpha)
        {
            // If no move has raised alpha, the evaluation is an upper bound.
            // (the exact score might be lower)
            type = UPPER_BOUND;
        }
        TT.add(pos, depth, type, bestMove, bestEval);

        // update the move to make if we are at the root node
        if (ply == 0)
        {
            moveToMake = bestMove;
        }
        return bestEval;
    }

    // TODO add concept of checkmate or stalemate?
    /**
     * Quiescence search to avoid horizon effect by evaluating only "quiet" positions.
     * Only capture moves are considered.
     *
     * @param pos The current position
     * @param alpha Represents the best score that the side to move can currently guarantee
     * @param beta Represents the best score that the opponent can currently guarantee
     * @return Evaluation of the position, positive means advantage for the side to move
     */
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
            return standPat;
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
            // perform move ordering
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
                return eval;
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
        score += MVV_LVA[typeOf(captured)][typeOf(piece)] * MVV_LVA_SCORE_MULTIPLIER;
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
