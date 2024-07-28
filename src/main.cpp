#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include "position.hpp"
#include "perft.hpp"
#include "bitboard.hpp"

using namespace std;

static const char DELIMITER = ' ';
static const string TEST_COMMAND = "test";
static const string POSITION_COMMAND = "position";
static const string PRINT_COMMAND = "print";
static const string PERFT_COMMAND = "perft";

void runPerftTest();

int main()
{
    engine::bitboard::init();
    engine::Position chessPos(engine::STARTING_FEN);

    string input;
    vector<string> tokens;
    size_t pos;

    while (true)
    {
        tokens.clear();
        cout << endl;
        getline(cin, input);
        while ((pos = input.find(DELIMITER)) != string::npos)
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
            string fen;
            for (size_t i = 1; i < tokens.size(); i++)
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
            chrono::steady_clock::time_point begin = chrono::steady_clock::now();
            engine::perft(chessPos, stoi(tokens[1]));
            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
        }
    }
    return 0;
}

void runPerftTest()
{
    vector<tuple<string, int, int>> perftTestCases = {
        make_tuple(engine::STARTING_FEN, 4, 197281),
        make_tuple("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603),
        make_tuple("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238),
        make_tuple("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333),
        make_tuple("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487),
        make_tuple("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594),
        make_tuple("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", 1, 8),
        make_tuple("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3", 1, 8),
        make_tuple("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", 1, 19),
        make_tuple("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", 1, 5),
        make_tuple("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", 1, 44),
        make_tuple("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", 1, 39),
        make_tuple("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", 1, 9),
        make_tuple("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379),
        make_tuple("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890),
        make_tuple("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888),
        make_tuple("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133),
        make_tuple("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467),
        make_tuple("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072),
        make_tuple("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711),
        make_tuple("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206),
        make_tuple("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476),
        make_tuple("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001),
        make_tuple("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658),
        make_tuple("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342),
        make_tuple("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683),
        make_tuple("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217),
        make_tuple("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584),
        make_tuple("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527),
    };

    for (const auto &testCase : perftTestCases)
    {
        string fen;
        int depth;
        uint64_t expected;
        tie(fen, depth, expected) = testCase;

        cout << endl
             << "Testing perft " << depth << " on " << fen << endl;
        engine::Position pos(fen);
        uint64_t actual = engine::perft(pos, depth, false);
        cout << "actual: " << actual << "\t\texpected: " << expected << endl;
        assert(actual == expected);
    }
}
