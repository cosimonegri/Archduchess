#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include <time.h>
#include "uci.hpp"
#include "bitboard.hpp"

int main()
{
    srand(time(0));
    engine::bitboard::init();

    engine::UCIEngine eng;
    eng.loop();

    return 0;
}
