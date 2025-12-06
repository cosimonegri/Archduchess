# Archduchess

A UCI chess engine written from scratch in C++. Hosted on GCP and rated ~2000 on Lichess.

[Play against Archduchess on Lichess!](https://lichess.org/@/ArchduchessBot)

<img alt="ArchduchessBotLichess" src="https://github.com/user-attachments/assets/f107574a-72ed-4e5d-96fd-e92bd9e63c04" />

## Building the Project

This project uses CMake for building.

### Prerequisites

- CMake >= 3.30
- A C++20 compatible compiler (e.g., GCC, Clang, MSVC)

### Build Types

- **Release**: Optimized for speed, disables debug info.
- **Debug**: No optimizations, includes debug symbols, extra warnings.
- **Profile**: Optimized for profiling, disables PIE, enables profiling flags.

### Option 1: Fast build

From the root folder

```sh
sh build.sh
```

The script accepts the following parameters:

| Parameter                       | Description                                                | Default   |
| ------------------------------- | ---------------------------------------------------------- | --------- |
| `release` / `debug` / `profile` | Choose build preset                                        | `release` |
| `--test`                        | Run tests after building                                   | `false`   |
| `--clean`                       | Delete the build directory before configuring and building | `false`   |

### Option 2: Build step by step

From the root folder

1. **Configure the project:**

   ```sh
   cmake --preset <configure-preset-name>
   ```

   Replace `<configure-preset-name>` with one of

   - ninja-release
   - ninja-debug
   - ninja-profile

2. **Build the project:**

   ```sh
   cmake --build --preset <build-preset-name>
   ```

   Replace `<build-preset-name>` with one of

   - release
   - debug
   - profile

   It has to match what you have chosen at step 1

3. **(optional) Run the tests:**

   ```sh
   ctest --preset <test-preset-name> --output-on-failure
   ```

   Replace `<test-preset-name>` with one of

   - release
   - debug
   - profile

   It has to match what you have chosen at step 1 and step 2

### Result

The engine executable will be in `build/<build-type>/bin/archduchess.exe`

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
- Aspiration Windows
- Transposition Table
- Quiescence Search
  - Delta Pruning
- Null Move Pruning
- Check Extension
- Late Move Reduction

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
