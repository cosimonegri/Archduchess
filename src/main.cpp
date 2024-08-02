#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include <time.h>
#include "engine/uci.hpp"
#include "engine/zobrist.hpp"
#include "engine/bitboard.hpp"

int main()
{
    srand(time(0));
    engine::bitboard::init();
    engine::zobrist::init();

    engine::UCIEngine eng;
    eng.loop();

    return 0;
}
