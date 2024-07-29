#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include "uci.hpp"
#include "bitboard.hpp"

int main()
{
    engine::bitboard::init();

    engine::UCIEngine eng;
    eng.loop();

    return 0;
}
