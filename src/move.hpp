#ifndef MOVE
#define MOVE

#include <iostream>
#include <cstdint>
#include "types.hpp"

namespace engine
{
    enum MoveFlag
    {
        QUIET,
        DOUBLE_PUSH,
        KING_CASTLE,
        QUEEN_CASTLE,
        CAPTURE,
        EN_PASSANT,
        KNIGHT_PROM = 8,
        BISHOP_PROM,
        ROOK_PROM,
        QUEEN_PROM,
        KNIGHT_PROM_CAPTURE,
        BISHOP_PROM_CAPTURE,
        ROOK_PROM_CAPTURE,
        QUEEN_PROM_CAPTURE,
    };

    class Move
    {
    private:
        uint16_t move;

    public:
        Move();
        Move(Tile from, Tile to, MoveFlag flag = QUIET);

        Tile getFrom() const;
        Tile getTo() const;
        MoveFlag getFlag() const;

        bool isCapture();
        bool isPromotion();
        bool isCastling();

        bool operator==(Move other) const { return move == other.move; }
        bool operator!=(Move other) const { return move != other.move; }

        friend std::ostream &operator<<(std::ostream &stream, const engine::Move &move)
        {
            stream << toString(move.getFrom()) << toString(move.getTo());
            return stream;
        };
    };

    constexpr int captureMask = CAPTURE << 12;
    constexpr int promotionMask = KNIGHT_PROM << 12;
}

#endif
