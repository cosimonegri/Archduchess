#include "move.hpp"

namespace engine
{
    Move::Move() : move(0) {};

    Move::Move(Tile from, Tile to, MoveFlag flag)
        : move(flag << 12 | to << 6 | from) {}

    Tile Move::getFrom() const
    {
        return Tile(move & 0x3f);
    }

    Tile Move::getTo() const
    {
        return Tile((move >> 6) & 0x3f);
    }

    MoveFlag Move::getFlag() const
    {
        return MoveFlag((move >> 12) & 0x0f);
    }

    bool Move::isCapture()
    {
        return (move & captureMask) != 0;
    }

    bool Move::isPromotion()
    {
        return (move & promotionMask) != 0;
    }

    bool Move::isCastling()
    {
        return (getFlag() == KING_CASTLE || getFlag() == QUEEN_CASTLE);
    }
}
