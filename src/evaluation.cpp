#include "evaluation.hpp"

namespace engine
{
    Eval evaluate(Position &pos)
    {
        Eval eval = 0;
        for (Tile tile = A1; tile <= H8; ++tile)
        {
            Piece piece = pos.getPiece(tile);
            if (piece != NULL_PIECE)
            {
                eval += getPieceEval(piece) * colorMult[colorOf(piece)];
                eval += getPiecePosEval(piece, tile) * colorMult[colorOf(piece)];
            }
        }
        return eval;
    }
}
