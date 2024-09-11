# Archduchess

A UCI chess engine written from scratch rated ~2050 on lichess.<br>
[Play against Archduchess on Lichess!](https://lichess.org/@/ArchduchessBot)

## Features

#### UCI Interface

- uci, isready, ucinewgame, position, go, stop, and quit commands
- go ponder, go searchmoves and go mate are not implemented

#### Move Generation

- Board representation using bitboards
- Bitboard shift for pawns
- Bitboard masks for knights and kings
- Magic bitboards for sliding pieces
- Generate pseudo-legal moves, then remove illegal moves

#### Evaluation

- Material count
- Piece-Square Tables

#### Search

- Negamax
- Alpha-Beta Pruning
- Iterative Deepening
- Transposition Table
- Quiescence Search
  - Delta Pruning
- Null Move Pruning
- Check Extensions

#### Move Ordering

- Best move from the previous iteration
- Hash move from the transposition table
- MVV-LVA
- 2 Killer Moves
- History Heuristic
  - Indexed by side to move, start square and end square

## Future Roadmap

- Better evaluation (pawn structure, king safety, endgame piece-square tables, etc...)
- Static Exchange Evaluation
- Late Move Reduction

## Helpful Resources

- [The Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
- [TalkChess Forum](https://talkchess.com/forum3/index.php)
- [Cute Chess](https://cutechess.com/)
- [Stockfish Features List](https://www.chessprogramming.org/Stockfish#Search)
