#include <string>
#include <thread>
#include "bot.hpp"
#include "search.hpp"
#include "position.hpp"
#include "generator.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include "types.hpp"

namespace engine
{
    std::string moveToUci(Move move)
    {
        std::string uciMove = toString(move.getFrom()) + toString(move.getTo());
        return move.isPromotion() ? uciMove + PROM_TO_CHAR.at(move.getFlag()) : uciMove;
    }

    std::string moveToSan(Position &pos, Move move)
    {
        if (move.raw() == 0)
            return "--";

        // castling
        if (move.getFlag() == KING_CASTLE)
            return "0-0";
        if (move.getFlag() == QUEEN_CASTLE)
            return "0-0-0";

        Piece piece = pos.getPiece(move.getFrom());
        assert(piece != NULL_PIECE);
        std::string san;

        if (typeOf(piece) != PAWN)
        {
            // moving piece
            san += std::toupper(PIECE_TO_CHAR.at(piece));

            // disambiguate
            Bitboard others = 0;
            MoveList moveList;
            generateMoves(pos, moveList);
            for (size_t i = 0; i < moveList.size; i++)
            {
                Move otherMove = moveList.moves[i];
                Piece otherPiece = pos.getPiece(otherMove.getFrom());
                if (typeOf(otherPiece) == typeOf(piece) &&
                    otherMove.getTo() == move.getTo() &&
                    otherMove.getFrom() != move.getFrom())
                {
                    others |= tileBB(otherMove.getFrom());
                }
            }
            bool sameFile = others & fileBB(fileOf(move.getFrom()));
            bool sameRank = others & rankBB(rankOf(move.getFrom()));
            if (!sameFile && sameRank)
                san += toString(fileOf(move.getFrom()));
            else if (sameFile && !sameRank)
                san += toString(rankOf(move.getFrom()));
            else if (sameFile && sameRank)
                san += toString(move.getFrom());
        }

        // capture
        if (pos.getPiece(move.getTo()) != NULL_PIECE)
        {
            if (typeOf(piece) == PAWN)
            {
                san += toString(fileOf(move.getFrom()));
            }
            san += 'x';
        }

        // target tile
        san += toString(move.getTo());

        // promotion
        if (move.isPromotion())
        {
            san += std::toupper(PROM_TO_CHAR.at(move.getFlag()));
        }

        // check
        RevertState state;
        pos.makeTurn(move, &state);
        if (pos.isKingInCheck())
        {
            san += '+';
        }
        pos.unmakeTurn();

        return san;
    }

    Bot::Bot() : pos{Position(START_FEN)}, SM{SearchManager()}, thinkSemaphore{0}
    {
        SM.setListener(this);
        thinkThread = std::thread(&Bot::runThinkThread, this);
    };

    Bot::~Bot()
    {
        thinkThread.join();
    };

    void Bot::setListener(MoveListener *listener)
    {
        this->listener = listener;
    }

    // todo maybe return a copy
    Position Bot::getPosition()
    {
        return pos;
    }

    void Bot::setPosition(std::string fen)
    {
        pos = Position(fen);
    }

    void Bot::makeTurn(std::string move)
    {
        if (move.size() < 4 || move.size() > 5)
        {
            // move format is not valid
            return;
        }
        Tile myFrom = makeTile(move.at(0), move.at(1));
        Tile myTo = makeTile(move.at(2), move.at(3));

        // instead of quiet, there could be any flag not related to promotions
        MoveFlag promFlag = QUIET;
        MoveFlag promCaptureFlag = QUIET;
        if (move.size() == 5)
        {
            switch (move.at(4))
            {
            case 'n':
                promFlag = KNIGHT_PROM;
                promCaptureFlag = KNIGHT_PROM_CAPTURE;
                break;
            case 'b':
                promFlag = BISHOP_PROM;
                promCaptureFlag = BISHOP_PROM_CAPTURE;
                break;
            case 'r':
                promFlag = ROOK_PROM;
                promCaptureFlag = ROOK_PROM_CAPTURE;
                break;
            case 'q':
                promFlag = QUEEN_PROM;
                promCaptureFlag = QUEEN_PROM_CAPTURE;
                break;
            default:
                // move format is not valid
                return;
            }
        }

        MoveList moveList;
        generateMoves(pos, moveList);
        for (size_t i = 0; i < moveList.size; i++)
        {
            Tile from = moveList.moves[i].getFrom();
            Tile to = moveList.moves[i].getTo();
            MoveFlag flag = moveList.moves[i].getFlag();
            bool isProm = moveList.moves[i].isPromotion();

            if (from == myFrom && to == myTo &&
                (!isProm || flag == promFlag || flag == promCaptureFlag))
            {
                pos.makeTurn(moveList.moves[i]);
            }
        };

        // move format is valid but the move is not valid
        return;
    }

    void Bot::startNewGame()
    {
        SM.clear();
    }

    void Bot::startThinking()
    {
        thinkSemaphore.release();
        std::thread cancelThread{
            [this]
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(MAX_THINK_TIME_MS));
                this->stopThinking();
            }};
        cancelThread.detach();
    }

    void Bot::stopThinking()
    {
        SM.setCancel();
    }

    void Bot::onSearchComplete(Move move)
    {
        listener->onMoveChosen(moveToUci(move));
    }

    void Bot::runThinkThread()
    {
        while (true)
        {
            thinkSemaphore.acquire();
            SM.startSearch(pos);
        }
    }

}
