#include "evaluation.hpp"

namespace engine
{
    int evaluate(Position &pos)
    {
        int eval = 0;
        for (Tile tile = A1; tile <= H8; ++tile)
        {
            Piece piece = pos.getPiece(tile);
            if (piece != NULL_PIECE)
            {
                eval += getPieceEval(piece);
                eval += getPiecePosEval(piece, tile);
            }
        }
        return eval;
    }
}
