#ifndef UTILS
#define UTILS

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <random>
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <vector>
#include "position.hpp"
#include "generator.hpp"
#include "zobrist.hpp"
#include "bitboard.hpp"
#include "types.hpp"

using namespace engine;

namespace utils
{
    /**
     * Function to generate good numbers fot the magic bitboards technique.
     * It reads these numbers from a "magics.txt" file (if some have already
     * been generate), tries to find better numbers, and then write them
     * back to the file.
     */
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

    /**
     * Function that pseudo-randomly generates some fen strings and writes
     * them to a "fens.txt" file. This is useful to test the efficiency of
     * the Zobrist hash keys.
     *
     * @param amount amount of fens to generate
     */
    void generateFens(uint64_t amount)
    {
        std::unordered_set<std::string> allFens;
        std::deque<std::string> currFens;
        std::string fen, newFen;

        allFens.insert(START_FEN);
        currFens.push_back(START_FEN);
        allFens.insert("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        currFens.push_back("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        allFens.insert("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        currFens.push_back("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        allFens.insert("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
        currFens.push_back("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
        allFens.insert("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        currFens.push_back("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        allFens.insert("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        currFens.push_back("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        allFens.insert("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2");
        currFens.push_back("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2");
        allFens.insert("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3");
        currFens.push_back("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3");
        allFens.insert("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2");
        currFens.push_back("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2");
        allFens.insert("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2");
        currFens.push_back("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2");
        allFens.insert("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2");
        currFens.push_back("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2");
        allFens.insert("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9");
        currFens.push_back("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9");
        allFens.insert("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4");
        currFens.push_back("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4");
        allFens.insert("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        currFens.push_back("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        allFens.insert("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        currFens.push_back("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        allFens.insert("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1");
        currFens.push_back("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1");

        std::random_device rnd;
        std::mt19937 gen(rnd());
        std::ofstream outputFile("fens.txt");

        uint64_t count = 0;
        while (count < amount)
        {
            fen = currFens.front();
            currFens.pop_front();
            Position pos = Position(fen);

            RevertState state1, state2;
            MoveList moveList;
            generateMoves<ALL>(pos, moveList);
            if (moveList.size == 0)
            {
                continue;
            }

            if (count % 50000 == 0 || count % 50000 == 1 || count % 50000 == 2)
            {
                std::shuffle(currFens.begin(), currFens.end(), gen);
                currFens.erase(currFens.begin(), currFens.begin() + (currFens.size() / 2));
            }

            int i = abs(rand()) % moveList.size;
            pos.makeTurn(moveList.moves[i], &state1);
            newFen = pos.getFen();
            if (!allFens.contains(newFen))
            {
                allFens.insert(newFen);
                currFens.push_back(newFen);
                outputFile << newFen << std::endl;
                count++;
            }
            pos.unmakeTurn();

            int j = abs(rand()) % moveList.size;
            pos.makeTurn(moveList.moves[j], &state2);
            newFen = pos.getFen();
            if (j != i && !allFens.contains(newFen))
            {
                allFens.insert(newFen);
                currFens.push_back(newFen);
                outputFile << newFen << std::endl;
                count++;
            }
            pos.unmakeTurn();

            int k = abs(rand()) % moveList.size;
            pos.makeTurn(moveList.moves[k]);
            newFen = pos.getFen();
            if (k != i && k != j && !allFens.contains(newFen))
            {
                allFens.insert(newFen);
                currFens.push_back(newFen);
                outputFile << newFen << std::endl;
                count++;
            }
        }

        outputFile.close();
    }

    void testZobrist()
    {
        std::unordered_set<Key> keys;
        uint64_t amount;
        std::string fen;

        std::cout << "How many fen to read from fens.txt: ";
        std::cin >> amount;
        std::ifstream inputFile("fens.txt");

        for (uint64_t count = 0; count < amount; count++)
        {
            std::getline(inputFile, fen);
            Position pos = Position(fen);
            keys.insert(pos.getZobristKey());
        }

        inputFile.close();
        std::cout << "Unique keys: " << keys.size() << std::endl;

        RevertState s1, s2, s3, s4, s5, s6, s7, s8, s9;
        Position pos = Position(START_FEN);
        uint64_t startKey = pos.getZobristKey();
        pos.makeTurn(Move(E2, E4, DOUBLE_PUSH), &s1);
        pos.makeTurn(Move(E7, E5, DOUBLE_PUSH), &s2);
        pos.makeTurn(Move(F1, D3), &s3);
        pos.makeTurn(Move(F8, A3), &s4);
        pos.makeTurn(Move(G1, F3), &s5);
        pos.makeTurn(Move(G8, H6), &s6);
        pos.makeTurn(Move(E1, G1, KING_CASTLE), &s7);
        pos.makeTurn(Move(E8, G8, KING_CASTLE), &s8);
        pos.makeTurn(Move(B1, A3), &s9);
        uint64_t finalKey = pos.getZobristKey();

        Position finalPos = Position("rnbq1rk1/pppp1ppp/7n/4p3/4P3/N2B1N2/PPPP1PPP/R1BQ1RK1 b - - 0 5");
        assert(finalKey == finalPos.getZobristKey());

        for (int i = 0; i < 9; i++)
            pos.unmakeTurn();

        assert(startKey == pos.getZobristKey());
    }
}

#endif
