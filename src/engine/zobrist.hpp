#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>
#include "types.hpp"

namespace engine
{
    extern Key pieceTileZ[15][64];
    extern Key castlingZ[16];
    extern Key enPassantFileZ[8];
    extern Key turnZ;

    namespace zobrist
    {
        void init();
    }

    inline Key getPieceTileZ(Piece piece, Tile tile)
    {
        return pieceTileZ[piece][tile];
    }

    inline Key getCastlingZ(CastlingRight c)
    {
        return castlingZ[c];
    }

    inline Key getEnPassantFileZ(Tile tile)
    {
        return enPassantFileZ[fileOf(tile)];
    }

    inline Key getTurnZ()
    {
        return turnZ;
    }
}

#endif
