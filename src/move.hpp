#ifndef MOVE
#define MOVE

#include <iostream>
#include <cstdint>
#include <cassert>
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

    constexpr int captureMask = CAPTURE << 12;
    constexpr int promotionMask = KNIGHT_PROM << 12;

    class Move
    {
    protected:
        uint16_t data;

    public:
        constexpr Move()
            : data(0) {};
        constexpr Move(Tile from, Tile to, MoveFlag flag = QUIET)
            : data(flag << 12 | to << 6 | from) {};

        constexpr Tile getFrom() const
        {
            assert(isValid());
            return Tile(data & 0x3f);
        }
        constexpr Tile getTo() const
        {
            assert(isValid());
            return Tile((data >> 6) & 0x3f);
        }

        constexpr MoveFlag getFlag() const
        {
            assert(isValid());
            return MoveFlag((data >> 12) & 0x0f);
        }

        constexpr bool isCapture() const
        {
            return (data & captureMask) != 0;
        }
        constexpr bool isPromotion() const
        {
            return (data & promotionMask) != 0;
        }
        constexpr bool isCastling() const
        {
            return (getFlag() == KING_CASTLE || getFlag() == QUEEN_CASTLE);
        }

        constexpr bool isValid() const { return data != 0; }

        constexpr uint16_t raw() const { return data; }

        constexpr bool operator==(Move other) const { return data == other.data; }
        constexpr bool operator!=(Move other) const { return data != other.data; }

        friend std::ostream &operator<<(std::ostream &stream, const engine::Move &move)
        {
            stream << toString(move.getFrom()) << toString(move.getTo());
            return stream;
        };
    };
}

#endif
