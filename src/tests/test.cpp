#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <tuple>
#include "bot.hpp"
#include "perft.hpp"
#include "position.hpp"
#include "zobrist.hpp"
#include "bitboard.hpp"

TEST_CASE("PerftTest", "[engine]")
{
    engine::bitboard::init();
    engine::zobrist::init();

    std::vector<std::tuple<std::string, int, uint64_t>> testCases = {
        std::make_tuple("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281),
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

    for (const auto &testCase : testCases)
    {
        std::string fen;
        int depth;
        uint64_t expected;
        std::tie(fen, depth, expected) = testCase;

        engine::Position pos(fen);
        uint64_t actual = engine::perft(pos, depth, false);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("BotTest", "[engine]")
{
    engine::bitboard::init();
    engine::zobrist::init();

    std::vector<std::tuple<std::string, std::string>> testCases = {
        // mate in 1
        std::make_tuple("5r1k/6pp/p1P5/1p1QB2n/3P4/P4pPq/7P/5RK1 b - - 1 31", "h3g2"),
        std::make_tuple("Q4nk1/p1p2pp1/7p/3P4/2BN1Rb1/2N1P1n1/PP3KP1/q7 b - - 0 21", "g3h1"),
        std::make_tuple("5r2/p1p3R1/2pk4/2Np3p/3Pp3/2P5/PP3rP1/6K1 w - - 0 39", "g7d7"),
        // mate in 2
        std::make_tuple("r1bq1rk1/p3bpp1/1p2p2p/2p5/3PN3/2PQPN1P/PPB2nP1/R5K1 w - - 0 19", "e4f6"),
        std::make_tuple("rn2k1nr/1pp2ppp/p7/8/4N3/3PQ3/PqP3PP/R3KB1R w KQkq - 0 15", "e4f6"),
        std::make_tuple("8/R3B3/6k1/6pp/6Pn/4P3/PP3P1K/6r1 b - - 2 33", "h4f3"),
    };

    for (const auto &testCase : testCases)
    {
        std::string fen;
        std::string move;
        std::tie(fen, move) = testCase;

        engine::Bot bot;
        bot.setPosition(fen);
        REQUIRE(bot.chooseMove() == move);
    }
}
