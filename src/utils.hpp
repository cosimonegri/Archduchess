#ifndef UTILS
#define UTILS

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include "types.hpp"
#include "bitboard.hpp"

using namespace engine;

namespace utils
{
    void generateMagics()
    {
        Bitboard orthogonalMagics[64];
        unsigned orthogonalShifts[64];
        Bitboard diagonalMagics[64];
        unsigned diagonalShifts[64];

        std::string number;
        std::ifstream inputFile("magics.txt");
        if (inputFile.is_open())
        {
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                orthogonalMagics[i] = std::stoull(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                orthogonalShifts[i] = std::stoi(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                diagonalMagics[i] = std::stoull(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                diagonalShifts[i] = std::stoi(number);
            }
            inputFile.close();
        }
        else
        {
            for (int i = 0; i < 64; i++)
            {
                orthogonalMagics[i] = 0;
                orthogonalShifts[i] = 0;
                diagonalMagics[i] = 0;
                diagonalShifts[i] = 0;
            }
        }

        std::random_device rnd;
        std::mt19937_64 gen(rnd());
        // std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
        std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

        std::unordered_set<Bitboard> keys;
        std::unordered_set<Bitboard> newKeys;
        std::vector<Bitboard> blockers;

        for (Tile tile = A1; tile <= H8; ++tile)
        {
            std::cout << "Generating magics for " << toString(tile) << std::endl;

            for (PieceType pt : {ROOK, BISHOP})
            {
                blockers.clear();
                bitboard::generateBlockers(pseudoAttacks[pt][tile], blockers);
                Bitboard *magics = pt == ROOK ? orthogonalMagics : diagonalMagics;
                unsigned *shifts = pt == ROOK ? orthogonalShifts : diagonalShifts;

                for (int i = 0; i < 10000; i++)
                {
                    Bitboard magic = dist(gen);
                    keys.clear();

                    bool betterMagic = true;
                    for (const auto &blocker : blockers)
                    {
                        Bitboard key = (blocker * magic) >> shifts[tile];
                        if (keys.contains(key))
                        {
                            betterMagic = false;
                            break;
                        }
                        keys.insert(key);
                    }

                    if (betterMagic)
                    {
                        unsigned shift = shifts[tile];
                        while (true)
                        {
                            newKeys.clear();
                            for (const auto &key : keys)
                            {
                                newKeys.insert(key >> 1);
                            }
                            if (newKeys.size() != keys.size())
                            {
                                break;
                            }
                            shift++;
                            keys.clear();
                            for (const auto &newKey : newKeys)
                            {
                                keys.insert(newKey);
                            }
                        }

                        magics[tile] = magic;
                        shifts[tile] = shift;
                    }
                }
            }
        }

        std::cout
            << "Best orthogonal bits count: "
            << (64 - *std::max_element(orthogonalShifts, orthogonalShifts + 64))
            << std::endl;
        std::cout
            << "Worst orthogonal bits count: "
            << (64 - *std::min_element(orthogonalShifts, orthogonalShifts + 64))
            << std::endl;
        std::cout
            << "Best diagonal bits count: "
            << (64 - *std::max_element(diagonalShifts, diagonalShifts + 64))
            << std::endl;
        std::cout
            << "Worst diagonal bits count: "
            << (64 - *std::min_element(diagonalShifts, diagonalShifts + 64))
            << std::endl;

        std::ofstream outputFile("magics.txt");
        for (int i = 0; i < 64; i++)
        {
            outputFile << orthogonalMagics[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << orthogonalShifts[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << diagonalMagics[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << diagonalShifts[i] << ",";
        }
        outputFile << std::endl;
        outputFile.close();
    }
}

#endif
