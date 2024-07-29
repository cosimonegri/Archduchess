#ifndef POSITION
#define POSITION

#include <string>
#include <map>
#include <stack>
#include "types.hpp"
#include "move.hpp"

namespace engine
{
    static const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    static const char FEN_DELIMITER = ' ';
    static const char FEN_RANKS_DELIMITER = '/';
    static const char FEN_EMPTY = '-';

    static const std::map<char, Piece> CHAR_TO_PIECE = {
        {'P', W_PAWN},
        {'N', W_KNIGHT},
        {'B', W_BISHOP},
        {'R', W_ROOK},
        {'Q', W_QUEEN},
        {'K', W_KING},
        {'p', B_PAWN},
        {'n', B_KNIGHT},
        {'b', B_BISHOP},
        {'r', B_ROOK},
        {'q', B_QUEEN},
        {'k', B_KING},
    };

    static const std::map<Piece, char> PIECE_TO_CHAR = {
        {W_PAWN, 'P'},
        {W_KNIGHT, 'N'},
        {W_BISHOP, 'B'},
        {W_ROOK, 'R'},
        {W_QUEEN, 'Q'},
        {W_KING, 'K'},
        {B_PAWN, 'p'},
        {B_KNIGHT, 'n'},
        {B_BISHOP, 'b'},
        {B_ROOK, 'r'},
        {B_QUEEN, 'q'},
        {B_KING, 'k'},
    };

    static const std::map<char, Color> CHAR_TO_COLOR = {
        {'w', WHITE},
        {'b', BLACK},
    };

    static const std::map<Color, char> COLOR_TO_CHAR = {
        {WHITE, 'w'},
        {BLACK, 'b'},
    };

    static const std::map<char, CastlingRight> CHAR_TO_CASTLING = {
        {'K', W_KING_SIDE},
        {'Q', W_QUEEN_SIDE},
        {'k', B_KING_SIDE},
        {'q', B_QUEEN_SIDE},
    };

    struct RevertState
    {
        Move move;
        CastlingRight castling;
        Tile enPassant;
        Piece captured;

        RevertState *previous;
    };

    class Position
    {
    private:
        Piece board[64];
        Bitboard typeBB[7];
        Bitboard colorBB[2];
        Color turn;
        CastlingRight castling;
        Tile enPassant;
        int halfMove;
        int fullMove;

        RevertState *state;

        void init();
        void setPiece(Tile tile, Piece piece);
        void clearPiece(Tile tile);
        void switchTurn();

    public:
        Position(const std::string &fen);

        Piece getPiece(Tile tile) const;
        Bitboard getPieces(PieceType pt) const;
        Bitboard getPieces(Color color) const;
        Bitboard getPieces(Piece piece) const;
        Bitboard getPieces() const;
        Bitboard getEmpty() const;

        Color getTurn() const;
        Tile getEnPassant() const;

        bool hasCastlingRight(CastlingRight c) const;
        bool castlingPathFree(CastlingRight c) const;
        Bitboard getCastlingKingPath(CastlingRight c) const;
        Tile getCastlingKingTo(CastlingRight c) const;

        template <Color C>
        Bitboard getAttacksBB() const;
        Bitboard getAttacksBB(Color color) const;

        void makeTurn(Move move, RevertState *newState = NULL);
        void unmakeTurn();
        void print();
    };
}

#endif
