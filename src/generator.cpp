#include "generator.hpp"
#include "bitboard.hpp"

namespace engine
{
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

    void generatePseudoPawnMoves(const Position &pos, MoveList &moveList)
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

    void generatePseudoMoves(PieceType pt, const Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard pieces = pos.getPieces(makePiece(pt, color));

        while (pieces != 0)
        {
            Tile from = popLsb(pieces);
            Bitboard attacks = getAttacksBB(pt, from, pos.getPieces()) & ~pos.getPieces(color);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    void generatePseudoKingMoves(const Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard king = pos.getPieces(makePiece(KING, color));
        Tile from = popLsb(king);
        Bitboard attacks = getAttacksBB(KING, from, pos.getPieces()) & ~pos.getPieces(color);
        while (attacks != 0)
        {
            Tile to = popLsb(attacks);
            moveList.moves[moveList.size++] = Move(from, to);
        }
        CastlingRight kingSide = color == WHITE ? W_KING_SIDE : B_KING_SIDE;
        CastlingRight queenSide = color == WHITE ? W_QUEEN_SIDE : B_QUEEN_SIDE;
        if (pos.hasCastlingRight(kingSide) && pos.castlingPathFree(kingSide))
        {
            moveList.moves[moveList.size++] = Move(from, pos.getCastlingKingTo(kingSide), KING_CASTLE);
        }
        if (pos.hasCastlingRight(queenSide) && pos.castlingPathFree(queenSide))
        {
            moveList.moves[moveList.size++] = Move(from, pos.getCastlingKingTo(queenSide), QUEEN_CASTLE);
        }
    }

    void generatePseudoMoves(const Position &pos, MoveList &moveList)
    {
        generatePseudoPawnMoves(pos, moveList);
        generatePseudoMoves(KNIGHT, pos, moveList);
        generatePseudoMoves(BISHOP, pos, moveList);
        generatePseudoMoves(ROOK, pos, moveList);
        generatePseudoMoves(QUEEN, pos, moveList);
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

            if (pseudoMoves.moves[i].getFlag() == KING_CASTLE)
            {
                CastlingRight kingSide = color == WHITE ? W_KING_SIDE : B_KING_SIDE;
                king |= pos.getCastlingKingPath(kingSide);
            }
            else if (pseudoMoves.moves[i].getFlag() == QUEEN_CASTLE)
            {
                CastlingRight queenSide = color == WHITE ? W_QUEEN_SIDE : B_QUEEN_SIDE;
                king |= pos.getCastlingKingPath(queenSide);
            }

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
