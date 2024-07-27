#include <iostream>
#include "position.hpp"
#include "bitboard.hpp"

namespace engine
{
    Position::Position(const std::string &fen)
        : typeBB{0, 0, 0, 0, 0, 0, 0}, colorBB{0, 0},
          castling(NULL_CASTLING), enPassant(NULL_TILE)
    {
        init();
        int index = 0;
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
                setPiece((Tile)tile, CHAR_TO_PIECE.at(c));
                tile++;
            };
            index++;
        }

        turn = CHAR_TO_COLOR.at(fen.at(index));
        index += 2;

        while (index < fen.length())
        {
            char c = fen.at(index);
            if (c == FEN_DELIMITER)
            {
                index++;
                break;
            }
            addCastling(castling, CHAR_TO_CASTLING.at(fen.at(index)));
            index++;
        }

        // todo finish
    }

    Piece Position::getPiece(Tile tile) const
    {
        assert(isValid(tile));
        return board[tile];
    }

    Bitboard Position::getPieces(PieceType pt) const
    {
        if (pt == NULL_TYPE)
        {
            return getEmpty();
        }
        return typeBB[pt - PAWN];
    }

    Bitboard Position::getPieces(Color color) const
    {
        return colorBB[color];
    }

    Bitboard Position::getPieces(Piece piece) const
    {
        if (piece == NULL_PIECE)
        {
            return getEmpty();
        }
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

    bool Position::canCastle(CastlingRight c) const
    {
        return (castling & c) == c;
    }

    Tile Position::getEnPassant() const
    {
        return enPassant;
    }

    void Position::makeTurn(Move move)
    {
        Tile from = move.getFrom();
        Tile to = move.getTo();
        MoveFlag flag = move.getFlag();

        revertHistory.push(new RevertState(move, castling, enPassant, board[to]));

        // remove castling right when the king moves
        if (typeOf(board[from]) == KING)
        {
            removeCastling(castling, getCastlingRights(turn));
        }

        // remove castling right when a rook moves or is captured
        if (from == A1 || to == A1)
        {
            removeCastling(castling, W_QUEEN_SIDE);
        }
        if (from == H1 || to == H1)
        {
            removeCastling(castling, W_KING_SIDE);
        }
        if (from == A8 || to == A8)
        {
            removeCastling(castling, B_QUEEN_SIDE);
        }
        if (from == H8 || to == H8)
        {
            removeCastling(castling, B_KING_SIDE);
        }

        // remove the captured piece if there is one, and execute the move
        if (board[to] != NULL_PIECE)
        {
            clearPiece(to);
        }
        setPiece(to, board[from]);
        clearPiece(from);

        enPassant = NULL_TILE;
        if (flag == QUIET)
        {
            switchTurn();
            return;
        }

        // handle other side effects
        if (flag == DOUBLE_PUSH)
        {
            enPassant = from + getPawnPushDir(turn);
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
    }

    void Position::unmakeTurn()
    {
        if (revertHistory.size() == 0)
        {
            return;
        }
        RevertState *state = revertHistory.top();
        revertHistory.pop();
        switchTurn();

        Tile from = (*state).move.getFrom();
        Tile to = (*state).move.getTo();
        MoveFlag flag = (*state).move.getFlag();

        // execute move, and add the captured piece if there is one
        setPiece(from, board[to]);
        clearPiece(to);
        if ((*state).captured != NULL_PIECE)
        {
            setPiece(to, (*state).captured);
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

        castling = (*state).castling;
        enPassant = (*state).enPassant;
        delete state;
    }

    void Position::print()
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
        std::cout << "    A   B   C   D   E   F   G   H  " << std::endl;
    }

    void Position::init()
    {
        for (Tile tile = A1; tile <= H8; ++tile)
        {
            board[tile] = NULL_PIECE;
        }
    }

    void Position::setPiece(Tile tile, Piece piece)
    {
        board[tile] = piece;
        setBit(typeBB[typeOf(piece)], tile);
        setBit(colorBB[colorOf(piece)], tile);
    }

    void Position::clearPiece(Tile tile)
    {
        Piece piece = board[tile];
        board[tile] = NULL_PIECE;
        clearBit(typeBB[typeOf(piece)], tile);
        clearBit(colorBB[colorOf(piece)], tile);
    }

    void Position::switchTurn()
    {
        turn = ~turn;
    }
}
