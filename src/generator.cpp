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

    template <Color C>
    void generatePseudoPawnMoves(const Position &pos, MoveList &moveList)
    {
        constexpr Color enemyColor = ~C;
        constexpr Direction pushDir = getPawnPushDir(C);
        constexpr Direction diagRight = getPawnRightDir(C);
        constexpr Direction diagLeft = getPawnLeftDir(C);
        constexpr Bitboard doubleRank = C == WHITE ? rank3 : rank6;
        constexpr Bitboard finalRank = C == WHITE ? rank7 : rank2;

        Bitboard pawns = pos.getPieces(PAWN, C);
        Bitboard empty = pos.getEmpty();
        Bitboard enemies = pos.getPieces(enemyColor);

        Bitboard promPawns = pawns & finalRank;
        Bitboard nonPromPawns = pawns & ~finalRank;

        // single and double pushes (without promotions)
        Bitboard pushes = shiftBB<pushDir>(nonPromPawns) & empty;
        Bitboard doublePushes = shiftBB<pushDir>(pushes & doubleRank) & empty;
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
        Bitboard attacksRight = shiftBB<diagRight>(nonPromPawns) & enemies;
        Bitboard attacksLeft = shiftBB<diagLeft>(nonPromPawns) & enemies;
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
        Bitboard promPushes = shiftBB<pushDir>(promPawns) & empty;
        Bitboard promAttacksRight = shiftBB<diagRight>(promPawns) & enemies;
        Bitboard promAttacksLeft = shiftBB<diagLeft>(promPawns) & enemies;
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
            Bitboard attackers = nonPromPawns & getPawnAttacksBB<enemyColor>(pos.getEnPassant());
            while (attackers)
            {
                Tile from = popLsb(attackers);
                moveList.moves[moveList.size++] = Move(from, pos.getEnPassant(), EN_PASSANT);
            }
        }
    }

    template <Color C, PieceType PT>
    void generatePseudoMoves(const Position &pos, MoveList &moveList)
    {
        Bitboard pieces = pos.getPieces(PT, C);

        while (pieces != 0)
        {
            Tile from = popLsb(pieces);
            Bitboard attacks = getAttacksBB<PT>(from, pos.getPieces()) & ~pos.getPieces(C);
            while (attacks != 0)
            {
                Tile to = popLsb(attacks);
                moveList.moves[moveList.size++] = Move(from, to);
            }
        }
    }

    // Generate all legal king moves
    template <Color C>
    void generateKingMoves(const Position &pos, MoveList &moveList)
    {
        Bitboard king = pos.getPieces(KING, C);
        Tile from = popLsb(king);
        Bitboard attackedTiles = pos.getAttacksBB(~C);
        Bitboard attacks = getAttacksBB<KING>(from, pos.getPieces()) & ~pos.getPieces(C) & ~attackedTiles;
        while (attacks != 0)
        {
            Tile to = popLsb(attacks);
            moveList.moves[moveList.size++] = Move(from, to);
        }

        CastlingRight kingSide = C == WHITE ? W_KING_SIDE : B_KING_SIDE;
        if (pos.hasCastlingRight(kingSide) &&
            pos.castlingPathFree(kingSide) &&
            (pos.getCastlingKingPath(kingSide) & attackedTiles) == 0)
        {
            moveList.moves[moveList.size++] = Move(from, pos.getCastlingKingTo(kingSide), KING_CASTLE);
        }

        CastlingRight queenSide = C == WHITE ? W_QUEEN_SIDE : B_QUEEN_SIDE;
        if (pos.hasCastlingRight(queenSide) &&
            pos.castlingPathFree(queenSide) &&
            (pos.getCastlingKingPath(queenSide) & attackedTiles) == 0)
        {
            moveList.moves[moveList.size++] = Move(from, pos.getCastlingKingTo(queenSide), QUEEN_CASTLE);
        }
    }

    // Generate all pseudo legal moves except for king moves
    template <Color C>
    void generatePseudoMoves(const Position &pos, MoveList &moveList)
    {
        generatePseudoPawnMoves<C>(pos, moveList);
        generatePseudoMoves<C, KNIGHT>(pos, moveList);
        generatePseudoMoves<C, BISHOP>(pos, moveList);
        generatePseudoMoves<C, ROOK>(pos, moveList);
        generatePseudoMoves<C, QUEEN>(pos, moveList);
    }

    void generateMoves(Position &pos, MoveList &moveList)
    {
        Color color = pos.getTurn();
        Bitboard king = pos.getPieces(KING, color);
        Tile kingTile = popLsb(king);

        RevertState state;
        MoveList pseudoMoves;

        color == WHITE ? generatePseudoMoves<WHITE>(pos, pseudoMoves)
                       : generatePseudoMoves<BLACK>(pos, pseudoMoves);

        for (size_t i = 0; i < pseudoMoves.size; i++)
        {
            pos.makeTurn(pseudoMoves.moves[i], &state);
            if (!pos.isTileAttackedBy(kingTile, ~color))
            {
                moveList.moves[moveList.size++] = pseudoMoves.moves[i];
            }
            pos.unmakeTurn();
        }

        color == WHITE ? generateKingMoves<WHITE>(pos, moveList)
                       : generateKingMoves<BLACK>(pos, moveList);
    }
}
