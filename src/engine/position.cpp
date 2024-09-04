#include <iostream>
#include <algorithm>
#include "position.hpp"
#include "bitboard.hpp"

namespace engine
{
    Position::Position(const std::string &fen)
        : typeBB{0, 0, 0, 0, 0, 0, 0}, colorBB{0, 0},
          castling(NULL_CASTLING), enPassant(NULL_TILE),
          zobristKey(0ULL)
    {
        for (Tile tile = A1; tile <= H8; ++tile)
            board[tile] = NULL_PIECE;

        size_t index = 0;
        int tile = A8;
        while (index < fen.length())
        {
            char c = fen.at(index);
            if (c == FEN_DELIMITER)
            {
                index++;
                break;
            }

            if (c == FEN_RANKS_DELIMITER)
            {
                tile -= 16;
            }
            else if (isdigit(c))
            {
                tile += (c - '0');
            }
            else
            {
                Piece piece = CHAR_TO_PIECE.at(c);
                board[tile] = piece;
                setBit(typeBB[typeOf(piece)], (Tile)tile);
                setBit(colorBB[colorOf(piece)], (Tile)tile);
                tile++;
            };
            index++;
        }

        turn = CHAR_TO_COLOR.at(fen.at(index));
        index += 2;

        while (index < fen.length())
        {
            char c = fen.at(index);
            if (c == FEN_EMPTY)
            {
                index += 2;
                break;
            }
            if (c == FEN_DELIMITER)
            {
                index++;
                break;
            }
            addCastling(castling, CHAR_TO_CASTLING.at(fen.at(index)));
            index++;
        }

        char c = fen.at(index);
        index++;
        if (c != FEN_EMPTY)
        {
            char fileChar = c;
            char rankChar = fen.at(index);
            index++;
            enPassant = makeTile(fileChar, rankChar);
        }
        index++;

        halfMove = 0;
        while (index < fen.length())
        {
            char c = fen.at(index);
            if (c == FEN_DELIMITER)
            {
                index++;
                break;
            }
            halfMove = 10 * halfMove + (c - '0');
            index++;
        }

        fullMove = 0;
        while (index < fen.length())
        {
            char c = fen.at(index);
            if (c == FEN_DELIMITER)
            {
                index++;
                break;
            }
            fullMove = 10 * fullMove + (c - '0');
            index++;
        }

        initZobristKey();
    }

    std::string Position::getFen() const
    {
        std::string fen;
        for (Rank rank = RANK_8; rank >= RANK_1; --rank)
        {
            int empty = 0;
            for (File file = FILE_A; file <= FILE_H; ++file)
            {
                Piece piece = getPiece(makeTile(file, rank));
                if (piece == NULL_PIECE)
                {
                    empty++;
                    continue;
                }
                if (empty != 0)
                {
                    fen += std::to_string(empty);
                    empty = 0;
                }
                fen += PIECE_TO_CHAR.at(piece);
            }
            if (empty != 0)
                fen += std::to_string(empty);
            if (rank != RANK_1)
                fen += FEN_RANKS_DELIMITER;
        }
        fen += FEN_DELIMITER;

        fen += COLOR_TO_CHAR.at(getTurn());
        fen += FEN_DELIMITER;

        for (CastlingRight c : {W_KING_SIDE, W_QUEEN_SIDE, B_KING_SIDE, B_QUEEN_SIDE})
        {
            if (hasCastlingRight(c))
                fen += CASTLING_TO_CHAR.at(c);
        }
        if (castling == NULL_CASTLING)
        {
            fen += FEN_EMPTY;
        }
        fen += FEN_DELIMITER;

        if (enPassant != NULL_TILE)
        {
            fen += toString(enPassant);
        }
        else
        {
            fen += FEN_EMPTY;
        }
        fen += FEN_DELIMITER;

        fen += std::to_string(halfMove);
        fen += FEN_DELIMITER;

        fen += std::to_string(fullMove);
        return fen;
    }

    Piece Position::getPiece(Tile tile) const
    {
        assert(isValid(tile));
        return board[tile];
    }

    Bitboard Position::getPieces(PieceType pt) const
    {
        assert(pt != NULL_TYPE);
        return typeBB[pt];
    }

    Bitboard Position::getPieces(Color color) const
    {
        return colorBB[color];
    }

    Bitboard Position::getPieces(PieceType pt, Color color) const
    {
        assert(pt != NULL_TYPE);
        return typeBB[pt] & colorBB[color];
    }

    Bitboard Position::getPieces(Piece piece) const
    {
        assert(piece != NULL_PIECE);
        return typeBB[typeOf(piece)] & colorBB[colorOf(piece)];
    }

    Bitboard Position::getPieces() const
    {
        return colorBB[WHITE] | colorBB[BLACK];
    }

    Bitboard Position::getEmpty() const
    {
        return ~getPieces();
    }

    Color Position::getTurn() const
    {
        return turn;
    }

    Tile Position::getEnPassant() const
    {
        return enPassant;
    }

    int Position::getHalfMove() const
    {
        return halfMove;
    }

    int Position::getFullMove() const
    {
        return fullMove;
    }

    bool Position::hasCastlingRight(CastlingRight c) const
    {
        return (castling & c) == c;
    }

    bool Position::castlingPathFree(CastlingRight c) const
    {
        Bitboard castlingPath = c == W_KING_SIDE    ? getBetweenBB(E1, G1)
                                : c == W_QUEEN_SIDE ? getBetweenBB(E1, B1)
                                : c == B_KING_SIDE  ? getBetweenBB(E8, G8)
                                : c == B_QUEEN_SIDE ? getBetweenBB(E8, B8)
                                                    : 0;
        return (castlingPath & getPieces()) == 0;
    }

    Bitboard Position::getCastlingKingPath(CastlingRight c) const
    {
        return c == W_KING_SIDE    ? getBetweenBB(H1, E1)
               : c == W_QUEEN_SIDE ? getBetweenBB(B1, E1)
               : c == B_KING_SIDE  ? getBetweenBB(H8, E8)
               : c == B_QUEEN_SIDE ? getBetweenBB(B8, E8)
                                   : 0;
    }

    Tile Position::getCastlingKingTo(CastlingRight c) const
    {
        return c == W_KING_SIDE    ? G1
               : c == W_QUEEN_SIDE ? C1
               : c == B_KING_SIDE  ? G8
               : c == B_QUEEN_SIDE ? C8
                                   : NULL_TILE;
    }

    template <Color C>
    Bitboard Position::getAttacksBB(bool excludeKingBlocker) const
    {
        constexpr Direction diagRight = getPawnRightDir(C);
        constexpr Direction diagLeft = getPawnLeftDir(C);

        Bitboard pawns = getPieces(PAWN, C);
        Bitboard attacks = 0;
        attacks |= shiftBB<diagRight>(pawns);
        attacks |= shiftBB<diagLeft>(pawns);

        Bitboard occupied = excludeKingBlocker
                                ? getPieces() & ~getPieces(KING, ~C)
                                : getPieces();
        for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
        {
            Bitboard pieces = getPieces(pt, C);
            while (pieces != 0)
            {
                Tile from = popLsb(pieces);
                attacks |= engine::getAttacksBB(pt, from, occupied);
            }
        }
        return attacks;
    }

    Bitboard Position::getAttacksBB(Color color, bool excludeKingBlocker) const
    {
        return color == WHITE
                   ? getAttacksBB<WHITE>(excludeKingBlocker)
                   : getAttacksBB<BLACK>(excludeKingBlocker);
    }

    bool Position::isTileAttackedBy(Tile tile, Color color) const
    {
        Bitboard allPieces = getPieces();
        for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
        {
            if ((engine::getAttacksBB(pt, tile, allPieces) & getPieces(pt, color)) != 0)
            {
                return true;
            }
        }
        Bitboard pawns = getPieces(PAWN, color);
        return color == WHITE
                   ? (getPawnAttacksBB<BLACK>(tile) & pawns) != 0
                   : (getPawnAttacksBB<WHITE>(tile) & pawns) != 0;
    }

    bool Position::isKingInCheck(Color color) const
    {
        Bitboard king = getPieces(KING, color);
        Tile kingTile = popLsb(king);
        return isTileAttackedBy(kingTile, ~color);
    }

    bool Position::isKingInCheck() const
    {
        return isKingInCheck(turn);
    }

    void Position::makeTurn(Move move, RevertState *newState)
    {
        Tile from = move.getFrom();
        Tile to = move.getTo();
        MoveFlag flag = move.getFlag();

        if (newState != NULL)
        {
            newState->move = move;
            newState->castling = castling;
            newState->enPassant = enPassant;
            newState->halfMove = halfMove;
            newState->captured = board[to];
            newState->zobristKey = zobristKey;
            newState->previous = state;
            state = newState;
        }

        halfMove += 1;
        if (board[to] != NULL_PIECE || typeOf(board[from]) == PAWN)
        {
            if (newState == NULL)
            {
                repetitions.clear();
            }
            halfMove = 0;
        }

        if (turn == BLACK)
        {
            fullMove += 1;
        }

        // remove castling right when the king moves
        if (typeOf(board[from]) == KING)
        {
            if (turn == WHITE && hasCastlingRight(W_QUEEN_SIDE))
                zobristKey ^= getCastlingZ(W_QUEEN_SIDE);
            if (turn == WHITE && hasCastlingRight(W_KING_SIDE))
                zobristKey ^= getCastlingZ(W_KING_SIDE);
            if (turn == BLACK && hasCastlingRight(B_QUEEN_SIDE))
                zobristKey ^= getCastlingZ(B_QUEEN_SIDE);
            if (turn == BLACK && hasCastlingRight(B_KING_SIDE))
                zobristKey ^= getCastlingZ(B_KING_SIDE);
            removeCastling(castling, getCastlingRights(turn));
        }

        // remove castling right when a rook moves or is captured
        if (from == A1 || to == A1)
        {
            if (hasCastlingRight(W_QUEEN_SIDE))
                zobristKey ^= getCastlingZ(W_QUEEN_SIDE);
            removeCastling(castling, W_QUEEN_SIDE);
        }
        if (from == H1 || to == H1)
        {
            if (hasCastlingRight(W_KING_SIDE))
                zobristKey ^= getCastlingZ(W_KING_SIDE);
            removeCastling(castling, W_KING_SIDE);
        }
        if (from == A8 || to == A8)
        {
            if (hasCastlingRight(B_QUEEN_SIDE))
                zobristKey ^= getCastlingZ(B_QUEEN_SIDE);
            removeCastling(castling, B_QUEEN_SIDE);
        }
        if (from == H8 || to == H8)
        {
            if (hasCastlingRight(B_KING_SIDE))
                zobristKey ^= getCastlingZ(B_KING_SIDE);
            removeCastling(castling, B_KING_SIDE);
        }

        // remove the captured piece if there is one, and execute the move
        if (board[to] != NULL_PIECE)
        {
            clearPiece(to);
        }
        setPiece(to, board[from]);
        clearPiece(from);

        if (enPassant != NULL_TILE)
        {
            zobristKey ^= getEnPassantFileZ(enPassant);
            enPassant = NULL_TILE;
        }

        // handle other side effects
        if (flag == DOUBLE_PUSH)
        {
            enPassant = from + getPawnPushDir(turn);
            zobristKey ^= getEnPassantFileZ(enPassant);
        }
        else if (flag == EN_PASSANT)
        {
            clearPiece(to - getPawnPushDir(turn));
        }
        else if (flag == KING_CASTLE)
        {
            Tile rookFrom = from + RIGHT + RIGHT + RIGHT;
            Tile rookTo = from + RIGHT;
            setPiece(rookTo, board[rookFrom]);
            clearPiece(rookFrom);
        }
        else if (flag == QUEEN_CASTLE)
        {
            Tile rookFrom = from + LEFT + LEFT + LEFT + LEFT;
            Tile rookTo = from + LEFT;
            setPiece(rookTo, board[rookFrom]);
            clearPiece(rookFrom);
        }
        else if (flag == QUEEN_PROM || flag == QUEEN_PROM_CAPTURE)
        {
            clearPiece(to);
            setPiece(to, makePiece(QUEEN, turn));
        }
        else if (flag == ROOK_PROM || flag == ROOK_PROM_CAPTURE)
        {
            clearPiece(to);
            setPiece(to, makePiece(ROOK, turn));
        }
        else if (flag == KNIGHT_PROM || flag == KNIGHT_PROM_CAPTURE)
        {
            clearPiece(to);
            setPiece(to, makePiece(KNIGHT, turn));
        }
        else if (flag == BISHOP_PROM || flag == BISHOP_PROM_CAPTURE)
        {
            clearPiece(to);
            setPiece(to, makePiece(BISHOP, turn));
        }

        switchTurn();
        repetitions.push_back(zobristKey);
    }

    void Position::unmakeTurn()
    {
        if (state == NULL)
        {
            return;
        }
        repetitions.pop_back();
        switchTurn();

        Tile from = state->move.getFrom();
        Tile to = state->move.getTo();
        MoveFlag flag = state->move.getFlag();

        // execute move, and add the captured piece if there is one
        setPiece(from, board[to]);
        clearPiece(to);
        if (state->captured != NULL_PIECE)
        {
            setPiece(to, state->captured);
        }

        if (flag == EN_PASSANT)
        {
            setPiece(to - getPawnPushDir(turn), makePiece(PAWN, ~turn));
        }
        else if (flag == KING_CASTLE)
        {
            Tile rookFrom = from + RIGHT;
            Tile rookTo = from + RIGHT + RIGHT + RIGHT;
            setPiece(rookTo, board[rookFrom]);
            clearPiece(rookFrom);
        }
        else if (flag == QUEEN_CASTLE)
        {
            Tile rookFrom = from + LEFT;
            Tile rookTo = from + LEFT + LEFT + LEFT + LEFT;
            setPiece(rookTo, board[rookFrom]);
            clearPiece(rookFrom);
        }
        else if ((*state).move.isPromotion())
        {
            clearPiece(from);
            setPiece(from, makePiece(PAWN, turn));
        }

        if (turn == BLACK)
        {
            fullMove -= 1;
        }

        castling = state->castling;
        enPassant = state->enPassant;
        halfMove = state->halfMove;
        zobristKey = state->zobristKey;
        state = state->previous;
    }

    bool Position::isRepeated() const
    {
        return repetitions.size() >= 3 &&
               std::count(repetitions.begin(), repetitions.end(), repetitions.back()) >= 3;
    }

    void Position::print() const
    {
        std::cout << std::endl;
        std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
        for (Rank rank = RANK_8; rank >= RANK_1; --rank)
        {
            std::cout << rank + 1 << " ";
            for (File file = FILE_A; file <= FILE_H; ++file)
            {
                Tile tile = makeTile(file, rank);
                if (board[tile] == NULL_PIECE)
                {
                    std::cout << "|   ";
                }
                else
                {
                    std::cout << "| " << PIECE_TO_CHAR.at(board[tile]) << " ";
                }
            }
            std::cout << "|" << std::endl;
            std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
        }
        std::cout << "    a   b   c   d   e   f   g   h  " << std::endl
                  << std::endl;
        std::cout << "Fen: " << getFen() << std::endl
                  << std::endl;
    }

    Key Position::getZobristKey() const
    {
        return zobristKey;
    }

    void Position::initZobristKey()
    {
        for (Tile tile = A1; tile <= H8; ++tile)
        {
            zobristKey ^= getPieceTileZ(getPiece(tile), tile);
        }
        if (turn == BLACK)
        {
            zobristKey ^= getTurnZ();
        }
        for (CastlingRight c : {W_KING_SIDE, W_QUEEN_SIDE, B_KING_SIDE, B_QUEEN_SIDE})
        {
            if (hasCastlingRight(c))
                zobristKey ^= getCastlingZ(c);
        }
        if (getEnPassant() != NULL_TILE)
        {
            zobristKey ^= getEnPassantFileZ(enPassant);
        }
    }

    void Position::setPiece(Tile tile, Piece piece)
    {
        zobristKey ^= getPieceTileZ(board[tile], tile);
        zobristKey ^= getPieceTileZ(piece, tile);
        board[tile] = piece;
        setBit(typeBB[typeOf(piece)], tile);
        setBit(colorBB[colorOf(piece)], tile);
    }

    void Position::clearPiece(Tile tile)
    {
        Piece piece = board[tile];
        zobristKey ^= getPieceTileZ(piece, tile);
        zobristKey ^= getPieceTileZ(NULL_PIECE, tile);
        board[tile] = NULL_PIECE;
        clearBit(typeBB[typeOf(piece)], tile);
        clearBit(colorBB[colorOf(piece)], tile);
    }

    void Position::switchTurn()
    {
        turn = ~turn;
        zobristKey ^= getTurnZ();
    }
}
