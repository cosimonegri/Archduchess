#include <iostream>
#include <string>
#include <chrono>
#include "position.hpp"
#include "perft.hpp"
#include "bitboard.hpp"

int main()
{
    engine::bitboard::init();
    engine::Position pos(engine::STARTING_FEN);
    // engine::bitboard::print(engine::pseudoAttacks[engine::ROOK][engine::A1]);
    // engine::bitboard::print(engine::pseudoAttacks[engine::ROOK][engine::B2]);
    // engine::bitboard::print(engine::pseudoAttacks[engine::ROOK][engine::D5]);
    // engine::bitboard::print(engine::pseudoAttacks[engine::BISHOP][engine::A1]);
    // engine::bitboard::print(engine::pseudoAttacks[engine::BISHOP][engine::B2]);
    // engine::bitboard::print(engine::pseudoAttacks[engine::BISHOP][engine::D5]);

    int depth;
    while (true)
    {
        std::cin >> depth;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        engine::perft(pos, depth);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
    }
    return 0;
}
