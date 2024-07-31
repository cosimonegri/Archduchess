#include <random>
#include "evaluation.hpp"
#include "bitboard.hpp"

namespace engine
{
    Eval evaluate(Position &pos)
    {
        Eval eval = 0;
        for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING})
        {
            for (Color color : {WHITE, BLACK})
            {
                Bitboard pieces = pos.getPieces(pt, color);
                while (pieces != 0)
                {
                    Tile tile = popLsb(pieces);
                    eval += getPieceEval(pt) * colorMult[color];
                    eval += getPiecePosEval(pt, color, tile) * colorMult[color];
                }
            }
        }
        return eval + (rand() % 5) - 2;
    }
}
