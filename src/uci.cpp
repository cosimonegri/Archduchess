#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include "uci.hpp"
#include "position.hpp"
#include "perft.hpp"
#include "misc.hpp"

namespace engine
{
    void UCIEngine::loop()
    {
        std::string command, token;

        while (true)
        {
            if (!std::getline(std::cin, command))
                command = "quit";

            // log(command);
            std::istringstream iss(command);
            token.clear();
            iss >> token;

            if (token == "uci")
                respond("uciok");

            else if (token == "isready")
                respond("readyok");

            else if (token == "ucinewgame")
                continue;

            else if (token == "position")
                processPosition(iss);

            else if (token == "go")
                processGo(iss);

            else if (token == "stop")
                continue;

            else if (token == "quit")
                break;

            else if (token == "d")
                printPosition();

            else if (token == "test")
                UCIEngine::runPerftTest();
        }
    }

    void UCIEngine::respond(std::string message)
    {
        std::cout << message << std::endl;
        // log(message);
    }

    void UCIEngine::processPosition(std::istringstream &iss)
    {
        std::string token, fen;
        iss >> token;

        if (token == "startpos")
        {
            fen = START_FEN;
            iss >> token;
        }
        else if (token == "fen")
        {
            while (iss >> token && token != "moves")
            {
                fen += token + " ";
            }
        }
        else
        {
            return;
        }

        bot.setPosition(fen);
        while (iss >> token)
        {
            bot.makeTurn(token);
        }
    }

    void UCIEngine::processGo(std::istringstream &iss)
    {
        std::string token;
        iss >> token;

        if (token == "perft")
        {
            iss >> token;
            try
            {
                int depth = std::stoi(token);
                runPerft(depth);
            }
            catch (...)
            {
            }
        }
        else
        {
            // todo engine should be able to read input while thinking
            respond("bestmove " + bot.chooseMove());
            return;
        }
    }

    void UCIEngine::printPosition()
    {
        bot.getPosition().print();
    }

    void UCIEngine::runPerft(int depth)
    {
        Position pos = bot.getPosition();
        auto begin = std::chrono::steady_clock::now();
        uint64_t nodes = perft(pos, depth);
        auto end = std::chrono::steady_clock::now();
        int64_t elapsedTime = getTimeMs(begin, end);

        std::cout << "\nNodes:\t" << nodes << std::endl;
        std::cout << "Time:\t" << elapsedTime << " ms" << std::endl;
        std::cout << "NPS:\t" << (nodes / elapsedTime) << "k" << std::endl
                  << std::endl;
    }

    void UCIEngine::runPerftTest()
    {
        std::vector<std::tuple<std::string, int, int>> perftTestCases = {
            std::make_tuple(START_FEN, 4, 197281),
            std::make_tuple("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603),
            std::make_tuple("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238),
            std::make_tuple("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333),
            std::make_tuple("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487),
            std::make_tuple("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594),
            std::make_tuple("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", 1, 8),
            std::make_tuple("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3", 1, 8),
            std::make_tuple("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", 1, 19),
            std::make_tuple("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", 1, 5),
            std::make_tuple("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", 1, 44),
            std::make_tuple("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", 1, 39),
            std::make_tuple("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", 1, 9),
            std::make_tuple("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379),
            std::make_tuple("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890),
            std::make_tuple("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888),
            std::make_tuple("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133),
            std::make_tuple("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467),
            std::make_tuple("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072),
            std::make_tuple("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711),
            std::make_tuple("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206),
            std::make_tuple("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476),
            std::make_tuple("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001),
            std::make_tuple("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658),
            std::make_tuple("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342),
            std::make_tuple("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683),
            std::make_tuple("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217),
            std::make_tuple("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584),
            std::make_tuple("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527),
        };

        for (const auto &testCase : perftTestCases)
        {
            std::string fen;
            int depth;
            uint64_t expected;
            std::tie(fen, depth, expected) = testCase;

            std::cout << std::endl
                      << "Testing perft " << depth << " on " << fen << std::endl;
            Position pos(fen);
            uint64_t actual = engine::perft(pos, depth, false);
            std::cout << "actual: " << actual << "\t\texpected: " << expected << std::endl;
            assert(actual == expected);
        }
        std::cout << std::endl;
    }
}
