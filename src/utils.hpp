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
        Bitboard rookMagics[64];
        unsigned rookShifts[64];
        Bitboard bishopMagics[64];
        unsigned bishopShifts[64];

        std::string number;
        std::ifstream inputFile("magics.txt");
        if (inputFile.is_open())
        {
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                rookMagics[i] = std::stoull(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                rookShifts[i] = std::stoi(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                bishopMagics[i] = std::stoull(number);
            }
            for (int i = 0; i < 64; i++)
            {
                std::getline(inputFile, number, ',');
                bishopShifts[i] = std::stoi(number);
            }
            inputFile.close();
        }
        else
        {
            for (int i = 0; i < 64; i++)
            {
                rookMagics[i] = 0;
                rookShifts[i] = 0;
                bishopMagics[i] = 0;
                bishopShifts[i] = 0;
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
                Bitboard *magics = pt == ROOK ? rookMagics : bishopMagics;
                unsigned *shifts = pt == ROOK ? rookShifts : bishopShifts;

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
            << "Best rook bits count: "
            << (64 - *std::max_element(rookShifts, rookShifts + 64))
            << std::endl;
        std::cout
            << "Worst rook bits count: "
            << (64 - *std::min_element(rookShifts, rookShifts + 64))
            << std::endl;
        std::cout
            << "Best bishop bits count: "
            << (64 - *std::max_element(bishopShifts, bishopShifts + 64))
            << std::endl;
        std::cout
            << "Worst bishop bits count: "
            << (64 - *std::min_element(bishopShifts, bishopShifts + 64))
            << std::endl;

        std::ofstream outputFile("magics.txt");
        for (int i = 0; i < 64; i++)
        {
            outputFile << rookMagics[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << rookShifts[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << bishopMagics[i] << ",";
        }
        outputFile << std::endl;
        for (int i = 0; i < 64; i++)
        {
            outputFile << bishopShifts[i] << ",";
        }
        outputFile << std::endl;
        outputFile.close();
    }
}

#endif
