#ifndef POSITION
#define POSITION

#include <string>
#include <map>
#include <stack>
#include "types.hpp"
#include "move.hpp"

namespace engine
{
    static const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

    static const std::map<Piece, std::string> PIECE_TO_SYMBOL = {
        {W_PAWN, "\u2659"},
        {W_KNIGHT, "\u2658"},
        {W_BISHOP, "\u2657"},
        {W_ROOK, "\u2656"},
        {W_QUEEN, "\u2655"},
        {W_KING, "\u2654"},
        {B_PAWN, "\u265F"},
        {B_KNIGHT, "\u265E"},
        {B_BISHOP, "\u265D"},
        {B_ROOK, "\u265C"},
        {B_QUEEN, "\u265B"},
        {B_KING, "\u265A"},
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

        RevertState(Move m, CastlingRight cast, Tile enp, Piece capt)
            : move{m}, castling{cast}, enPassant{enp}, captured{capt} {}
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

        std::stack<RevertState *> revertHistory;

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
        bool canCastle(CastlingRight c) const;
        Tile getEnPassant() const;

        void makeTurn(Move move);
        void unmakeTurn();
        void print();
    };
}

#endif
