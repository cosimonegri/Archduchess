#ifndef TIME_H
#define TIME_H

#include <chrono>
#include "types.hpp"

namespace engine
{
    constexpr uint64_t MAX_THINK_TIME_MS = 100000000;
    constexpr uint64_t STD_THINK_TIME_MS = 100;

    constexpr int DEFAULT_MOVESTOGO = 30;

    uint64_t calcThinkTimeMs(ThinkInfo info, Color side);

    bool shouldStop(ThinkInfo *info, Depth depth, uint64_t nodes,
                    std::chrono::_V2::steady_clock::time_point endTime);
}

#endif
