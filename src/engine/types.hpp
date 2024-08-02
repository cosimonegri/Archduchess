#ifndef TYPES
#define TYPES

#include <string>
#include <cstdint>
#include <cassert>

namespace engine
{
    using Bitboard = uint64_t;

    enum Color
    {
        WHITE,
        BLACK,
    };

    constexpr Color operator~(Color color)
    {
        return Color(color ^ 1);
    }

    enum PieceType
    {
        NULL_TYPE,
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
    };

    enum Piece
    {
        NULL_PIECE,
        W_PAWN = PAWN,
        W_KNIGHT,
        W_BISHOP,
        W_ROOK,
        W_QUEEN,
        W_KING,
        B_PAWN = PAWN + 8,
        B_KNIGHT,
        B_BISHOP,
        B_ROOK,
        B_QUEEN,
        B_KING,
    };

    constexpr Piece makePiece(PieceType pt, Color color)
    {
        return pt == NULL_TYPE ? NULL_PIECE : Piece((color << 3) | pt);
    }

    constexpr PieceType typeOf(Piece piece)
    {
        return PieceType(piece & 7);
    }

    constexpr Color colorOf(Piece piece)
    {
        assert(piece != NULL_PIECE);
        return Color((piece >> 3));
    }

    enum CastlingRight
    {
        NULL_CASTLING,
        W_KING_SIDE,
        W_QUEEN_SIDE = W_KING_SIDE << 1,
        B_KING_SIDE = W_KING_SIDE << 2,
        B_QUEEN_SIDE = W_KING_SIDE << 3,

        W_CASTLE = W_KING_SIDE | W_QUEEN_SIDE,
        B_CASTLE = B_KING_SIDE | B_QUEEN_SIDE,
        KING_SIDE = W_KING_SIDE | B_KING_SIDE,
        QUEEN_SIDE = W_QUEEN_SIDE | B_QUEEN_SIDE,

        ALL_CASTLING = W_CASTLE | B_CASTLE,
    };

    constexpr CastlingRight getCastlingRights(Color color)
    {
        return color == WHITE ? W_CASTLE : B_CASTLE;
    }

    inline void addCastling(CastlingRight &a, CastlingRight b)
    {
        a = static_cast<CastlingRight>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline void removeCastling(CastlingRight &a, CastlingRight b)
    {
        a = static_cast<CastlingRight>(static_cast<int>(a) & ~static_cast<int>(b));
    }

    // clang-format off
    enum Tile
    {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        NULL_TILE,
    };
    // clang-format on

    constexpr bool isValid(Tile tile)
    {
        return tile >= A1 && tile <= H8;
    }

    enum File
    {
        FILE_A,
        FILE_B,
        FILE_C,
        FILE_D,
        FILE_E,
        FILE_F,
        FILE_G,
        FILE_H,
    };

    enum Rank
    {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8,
    };

    constexpr Tile makeTile(File file, Rank rank)
    {
        return Tile((rank << 3) | file);
    }

    inline File fileOf(Tile tile)
    {
        assert(isValid(tile));
        return File(tile & 7);
    }

    inline Rank rankOf(Tile tile)
    {
        assert(isValid(tile));
        return Rank(tile >> 3);
    }

    inline Tile makeTile(char file, char rank)
    {
        assert(file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8');
        return makeTile(File(file - 'a'), Rank(rank - '1'));
    }

    inline std::string toString(Tile tile)
    {
        return (char)('a' + fileOf(tile)) + std::to_string(rankOf(tile) + 1);
    }

#define ENABLE_INCR_OPERATORS_ON(T)                          \
    inline T &operator++(T &t) { return t = T(int(t) + 1); } \
    inline T &operator--(T &t) { return t = T(int(t) - 1); }

    ENABLE_INCR_OPERATORS_ON(Tile)
    ENABLE_INCR_OPERATORS_ON(File)
    ENABLE_INCR_OPERATORS_ON(Rank)
#undef ENABLE_INCR_OPERATORS_ON

    enum Direction
    {
        UP = 8,
        DOWN = -8,
        RIGHT = 1,
        LEFT = -1,

        UP_RIGHT = UP + RIGHT,
        UP_LEFT = UP + LEFT,
        DOWN_RIGHT = DOWN + RIGHT,
        DOWN_LEFT = DOWN + LEFT,
    };

    constexpr Direction getPawnPushDir(Color color)
    {
        return color == WHITE ? UP : DOWN;
    }

    constexpr Direction getPawnRightDir(Color color)
    {
        return color == WHITE ? UP_RIGHT : DOWN_RIGHT;
    }

    constexpr Direction getPawnLeftDir(Color color)
    {
        return color == WHITE ? UP_LEFT : DOWN_LEFT;
    }

    // these operations might return something that is not a valid tile
    inline Tile operator+(Tile tile, Direction dir)
    {
        return Tile(int(tile) + int(dir));
    }
    inline Tile operator-(Tile tile, Direction dir)
    {
        return Tile(int(tile) - int(dir));
    }
    inline Tile &operator+=(Tile &tile, Direction dir)
    {
        return tile = tile + dir;
    }
    inline Tile &operator-=(Tile &tile, Direction dir)
    {
        return tile = tile - dir;
    }

    enum SlidingDir
    {
        ORTHOGONAL,
        DIAGONAL,
    };
}

#endif
