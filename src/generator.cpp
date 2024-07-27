#include "generator.hpp"
#include "bitboard.hpp"

namespace engine
{
    // todo see if this works and if is needed
    bool MoveList::contains(Move move)
    {
        for (int i = 0; i < size; i++)
        {
            if (moves[i] == move)
            {
                return true;
            }
        }
        return false;
    }

    void MoveList::clear()
    {
        size = 0;
    }

    void generatePromotionMoves(MoveList &moveList, Tile from, Tile to, bool isCapture)
    {
        moveList.moves[moveList.size++] = Move(from, to, isCapture ? QUEEN_PROM_CAPTURE : QUEEN_PROM);
        moveList.moves[moveList.size++] = Move(from, to, isCapture ? ROOK_PROM_CAPTURE : ROOK_PROM);
        moveList.moves[moveList.size++] = Move(from, to, isCapture ? BISHOP_PROM_CAPTURE : BISHOP_PROM);
        moveList.moves[moveList.size++] = Move(from, to, isCapture ? KNIGHT_PROM_CAPTURE : KNIGHT_PROM);
    }

    void generatePseudoPawnMoves(Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard pawns = pos.getPieces(makePiece(PAWN, color));
        Bitboard empty = pos.getEmpty();
        Bitboard enemies = pos.getPieces(~color);

        Direction pushDir = color == WHITE ? UP : DOWN;
        Direction diagRight = color == WHITE ? UP_RIGHT : DOWN_RIGHT;
        Direction diagLeft = color == WHITE ? UP_LEFT : DOWN_LEFT;

        Bitboard doubleRank = color == WHITE ? rank3 : rank6;
        Bitboard finalRank = color == WHITE ? rank7 : rank2;
        Bitboard promPawns = pawns & finalRank;
        Bitboard nonPromPawns = pawns & ~finalRank;

        // single and double pushes (without promotions)
        Bitboard pushes = shiftBB(nonPromPawns, pushDir) & empty;
        Bitboard doublePushes = shiftBB(pushes & doubleRank, pushDir) & empty;
        while (pushes)
        {
            Tile to = popLsb(pushes);
            moveList.moves[moveList.size++] = Move(to - pushDir, to);
        }
        while (doublePushes)
        {
            Tile to = popLsb(doublePushes);
            moveList.moves[moveList.size++] = Move(to - pushDir - pushDir, to, DOUBLE_PUSH);
        }

        // attacks (without promotions)
        Bitboard attacksRight = shiftBB(nonPromPawns, diagRight) & enemies;
        Bitboard attacksLeft = shiftBB(nonPromPawns, diagLeft) & enemies;
        while (attacksRight)
        {
            Tile to = popLsb(attacksRight);
            moveList.moves[moveList.size++] = Move(to - diagRight, to, CAPTURE);
        }
        while (attacksLeft)
        {
            Tile to = popLsb(attacksLeft);
            moveList.moves[moveList.size++] = Move(to - diagLeft, to, CAPTURE);
        }

        // promotions
        Bitboard promPushes = shiftBB(promPawns, pushDir) & empty;
        Bitboard promAttacksRight = shiftBB(promPawns, diagRight) & enemies;
        Bitboard promAttacksLeft = shiftBB(promPawns, diagLeft) & enemies;
        while (promPushes)
        {
            Tile to = popLsb(promPushes);
            generatePromotionMoves(moveList, to - pushDir, to, false);
        }
        while (promAttacksRight)
        {
            Tile to = popLsb(promAttacksRight);
            generatePromotionMoves(moveList, to - diagRight, to, true);
        }
        while (promAttacksLeft)
        {
            Tile to = popLsb(promAttacksLeft);
            generatePromotionMoves(moveList, to - diagLeft, to, true);
        }

        // en passant
        if (pos.getEnPassant() != NULL_TILE)
        {
            Bitboard attackers = nonPromPawns & pawnAttacks[~color][pos.getEnPassant()];
            while (attackers)
            {
                Tile from = popLsb(attackers);
                moveList.moves[moveList.size++] = Move(from, pos.getEnPassant(), CAPTURE);
            }
        }
    }

    void generatePseudoKnightMoves(Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard knights = pos.getPieces(makePiece(KNIGHT, color));

        while (knights != 0)
        {
            Tile from = popLsb(knights);
            Bitboard attacks = pseudoAttacks[KNIGHT][from] & ~pos.getPieces(color);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    void generatePseudoKingMoves(Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard king = pos.getPieces(makePiece(KING, color));

        while (king != 0)
        {
            Tile from = popLsb(king);
            Bitboard attacks = pseudoAttacks[KING][from] & ~pos.getPieces(color);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    void generatePseudoOrthogonalMoves(Position &pos, MoveList &moveList, PieceType pt)
    {
        Color color = pos.getTurn();
        Bitboard pieces = pos.getPieces(makePiece(pt, color));

        while (pieces != 0)
        {
            Tile from = popLsb(pieces);
            Bitboard blockers = pos.getPieces() & pseudoAttacks[ROOK][from];
            unsigned lookupKey = (blockers * ORTHOGONAL_MAGIC_NUMBERS[from]) >> ORTHOGONAL_SHIFTS[from];
            Bitboard attacks = orthogonalAttacks[from][lookupKey] & ~pos.getPieces(color);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    void generatePseudoDiagonalMoves(Position &pos, MoveList &moveList, PieceType pt)
    {
        Color color = pos.getTurn();
        Bitboard pieces = pos.getPieces(makePiece(pt, color));

        while (pieces != 0)
        {
            Tile from = popLsb(pieces);
            Bitboard blockers = pos.getPieces() & pseudoAttacks[BISHOP][from];
            unsigned lookupKey = (blockers * DIAGONAL_MAGIC_NUMBERS[from]) >> DIAGONAL_SHIFTS[from];
            Bitboard attacks = diagonalAttacks[from][lookupKey] & ~pos.getPieces(color);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    void generatePseudoMoves(Position &pos, MoveList &moveList)
    {
        generatePseudoPawnMoves(pos, moveList);
        generatePseudoKnightMoves(pos, moveList);
        generatePseudoDiagonalMoves(pos, moveList, BISHOP);
        generatePseudoOrthogonalMoves(pos, moveList, ROOK);
        generatePseudoDiagonalMoves(pos, moveList, QUEEN);
        generatePseudoOrthogonalMoves(pos, moveList, QUEEN);
        generatePseudoKingMoves(pos, moveList);
    }

    void generateMoves(Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        MoveList pseudoMoves, opponentMoves;
        generatePseudoMoves(pos, pseudoMoves);

        for (int i = 0; i < pseudoMoves.size; i++)
        {
            pos.makeTurn(pseudoMoves.moves[i]);
            Bitboard king = pos.getPieces(makePiece(KING, color));
            generatePseudoMoves(pos, opponentMoves);

            bool legal = true;
            for (int j = 0; j < opponentMoves.size; j++)
            {
                if ((tileBB(opponentMoves.moves[j].getTo()) & king) != 0)
                {
                    legal = false;
                    break;
                }
            }

            if (legal)
            {
                moveList.moves[moveList.size++] = pseudoMoves.moves[i];
            }
            opponentMoves.clear();
            pos.unmakeTurn();
        }
    }
}
