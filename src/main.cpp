#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cassert>
#include "position.hpp"
#include "perft.hpp"
#include "bitboard.hpp"

static const char DELIMITER = ' ';
static const std::string TEST_COMMAND = "test";
static const std::string POSITION_COMMAND = "position";
static const std::string PRINT_COMMAND = "print";
static const std::string PERFT_COMMAND = "perft";

void runPerftTest();

int main()
{
    engine::bitboard::init();
    engine::Position chessPos(engine::STARTING_FEN);

    std::string input;
    std::vector<std::string> tokens;
    size_t pos;

    while (true)
    {
        tokens.clear();
        std::cout << std::endl;
        std::getline(std::cin, input);
        while ((pos = input.find(DELIMITER)) != std::string::npos)
        {
            tokens.push_back(input.substr(0, pos));
            input.erase(0, pos + 1);
        }
        tokens.push_back(input);

        if (tokens.size() == 0)
            continue;

        if (tokens[0].compare(TEST_COMMAND) == 0)
        {
            runPerftTest();
        }

        else if (tokens[0].compare(POSITION_COMMAND) == 0)
        {
            if (tokens.size() < 2)
                continue;
            std::string fen;
            for (int i = 1; i < tokens.size(); i++)
            {
                if (i != 1)
                    fen += DELIMITER;
                fen += tokens[i];
            }
            chessPos = engine::Position(fen);
        }

        else if (tokens[0].compare(PRINT_COMMAND) == 0)
        {
            chessPos.print();
        }

        else if (tokens[0].compare(PERFT_COMMAND) == 0)
        {
            if (tokens.size() < 2)
                continue;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            engine::perft(chessPos, std::stoi(tokens[1]));
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
        }
    }
    return 0;
}

void runPerftTest()
{
    std::string fens[6] = {
        engine::STARTING_FEN,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    };

    uint64_t perftResults[6][4] = {
        {20, 400, 8902, 197281},
        {48, 2039, 97862, 4085603},
        {14, 191, 2812, 43238},
        {6, 264, 9467, 422333},
        {44, 1486, 62379, 2103487},
        {46, 2079, 89890, 3894594},
    };

    for (int i = 0; i < 6; i++)
    {
        engine::Position pos(fens[i]);
        std::cout << "Testing " << fens[i] << std::endl;
        for (int j = 0; j < 4; j++)
        {
            uint64_t actual = engine::perft(pos, j + 1, false);
            uint64_t expected = perftResults[i][j];
            std::cout << "actual: " << actual << "\t\texpected: " << expected << std::endl;
            assert(actual == expected);
        }
    }
}
