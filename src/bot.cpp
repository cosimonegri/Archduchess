#include "bot.hpp"
#include "position.hpp"
#include "generator.hpp"
#include "types.hpp"

namespace engine
{
    Bot::Bot() : pos{Position(START_FEN)} {};

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
}
