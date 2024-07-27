#include <iostream>
#include <vector>
#include "bitboard.hpp"

namespace engine
{
    int manhattanDistance[64][64];

    Bitboard pawnAttacks[2][64];
    Bitboard pseudoAttacks[7][64];
    Bitboard diagonalAttacks[64][16384];
    Bitboard orthogonalAttacks[64][16384];

    namespace bitboard
    {
        void generateBlockers(Bitboard movementMask, std::vector<Bitboard> &blockers)
        {
            std::vector<int> tileIndexes;
            while (movementMask != 0)
            {
                tileIndexes.push_back(popLsb(movementMask));
            }

            int blockersCount = 1 << tileIndexes.size();
            for (int blockerIndex = 0; blockerIndex < blockersCount; blockerIndex++)
            {
                Bitboard blocker = 0;
                for (int i = 0; i < tileIndexes.size(); i++)
                {
                    Bitboard bit = ((Bitboard)blockerIndex >> i) & 1;
                    blocker |= (bit << tileIndexes[i]);
                }
                blockers.push_back(blocker);
            }
        }

        Bitboard getSlidingAttacks(Tile tile, Bitboard blockers, SlidingDir slide)
        {
            Bitboard attacks = 0;
            Direction orthDirs[4] = {UP, DOWN, RIGHT, LEFT};
            Direction diagDirs[4] = {UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT};
            for (Direction dir : slide == ORTHOGONAL ? orthDirs : diagDirs)
            {
                Tile to = tile + dir;
                while (
                    isValid(to) &&
                    !((dir == RIGHT || dir == UP_RIGHT || dir == DOWN_RIGHT) && fileOf(to) == FILE_A) &&
                    !((dir == LEFT || dir == UP_LEFT || dir == DOWN_LEFT) && fileOf(to) == FILE_H))
                {
                    attacks |= tileBB(to);
                    if ((tileBB(to) & blockers) != 0)
                    {
                        break;
                    }
                    to += dir;
                }
            }
            return attacks;
        }

        void init()
        {
            for (Tile tileA = A1; tileA <= H8; ++tileA)
            {
                for (Tile tileB = A1; tileB <= H8; ++tileB)
                {
                    manhattanDistance[tileA][tileB] =
                        abs(fileOf(tileA) - fileOf(tileB)) + abs(rankOf(tileA) - rankOf(tileB));
                }
            }

            for (Tile tile = A1; tile <= H8; ++tile)
            {
                // generate pawn attack mask
                pawnAttacks[WHITE][tile] =
                    shiftBB(tileBB(tile), UP_RIGHT) | shiftBB(tileBB(tile), UP_LEFT);
                pawnAttacks[BLACK][tile] =
                    shiftBB(tileBB(tile), DOWN_RIGHT) | shiftBB(tileBB(tile), DOWN_LEFT);

                // generate knight attack mask
                for (int dir : {-17, -15, -10, -6, 6, 10, 15, 17})
                {
                    Tile to = Tile(tile + dir);
                    if (isValid(to) && manhattanDistance[tile][to] == 3)
                    {
                        pseudoAttacks[KNIGHT][tile] |= tileBB(to);
                    }
                }

                // generate bishop attack mask
                for (Direction dir : {UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT})
                {
                    Tile to = tile + dir;
                    while (
                        isValid(to) &&
                        !((dir == UP_RIGHT || dir == DOWN_RIGHT) && fileOf(to) == FILE_A) &&
                        !((dir == UP_LEFT || dir == DOWN_LEFT) && fileOf(to) == FILE_H))
                    {
                        pseudoAttacks[BISHOP][tile] |= tileBB(to);
                        to += dir;
                    }
                    pseudoAttacks[BISHOP][tile] &= ~tileBB(to - dir);
                }

                // generate rook attack mask
                for (Direction dir : {UP, DOWN, RIGHT, LEFT})
                {
                    Tile to = tile + dir;
                    while (
                        isValid(to) &&
                        !(dir == RIGHT && fileOf(to) == FILE_A) &&
                        !(dir == LEFT && fileOf(to) == FILE_H))
                    {
                        pseudoAttacks[ROOK][tile] |= tileBB(to);
                        to += dir;
                    }
                    pseudoAttacks[ROOK][tile] &= ~tileBB(to - dir);
                }

                // generate king attack mask
                for (Direction dir : {UP, DOWN, RIGHT, LEFT, UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT})
                {
                    pseudoAttacks[KING][tile] |= shiftBB(tileBB(tile), dir);
                }
            }

            // generate diagonal magic table
            for (Tile tile = A1; tile <= H8; ++tile)
            {
                std::vector<Bitboard> blockers;
                generateBlockers(pseudoAttacks[BISHOP][tile], blockers);
                for (const auto &blocker : blockers)
                {
                    Bitboard attacks = getSlidingAttacks(tile, blocker, DIAGONAL);
                    size_t lookupKey = (blocker * DIAGONAL_MAGIC_NUMBERS[tile]) >> DIAGONAL_SHIFTS[tile];
                    diagonalAttacks[tile][lookupKey] = attacks;
                }
            }

            // generate orthogonal magic table
            for (Tile tile = A1; tile <= H8; ++tile)
            {
                std::vector<Bitboard> blockers;
                generateBlockers(pseudoAttacks[ROOK][tile], blockers);
                for (const auto &blocker : blockers)
                {
                    Bitboard attacks = getSlidingAttacks(tile, blocker, ORTHOGONAL);
                    size_t lookupKey = (blocker * ORTHOGONAL_MAGIC_NUMBERS[tile]) >> ORTHOGONAL_SHIFTS[tile];
                    orthogonalAttacks[tile][lookupKey] = attacks;
                }
            }

            // todo free memory in magic tables
        }

        void print(Bitboard b)
        {
            std::cout << std::endl;
            std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
            for (Rank rank = RANK_8; rank >= RANK_1; --rank)
            {
                std::cout << rank + 1 << " ";
                for (File file = FILE_A; file <= FILE_H; ++file)
                {
                    Tile tile = makeTile(file, rank);
                    if ((b & tileBB(tile)) == 0)
                    {
                        std::cout << "|   ";
                    }
                    else
                    {
                        std::cout << "| X ";
                    }
                }
                std::cout << "|" << std::endl;
                std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
            }
            std::cout << "    A   B   C   D   E   F   G   H  " << std::endl;
        }
    }
}
