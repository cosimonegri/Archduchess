#include <limits>
#include "time.hpp"

namespace engine
{
    uint64_t calcThinkTimeMs(ThinkInfo info, Color side)
    {
        if (info.flags & (F_INFINITE | F_DEPTH | F_NODES))
            return INF_THINK_TIME_MS;

        if (info.flags & F_MOVETIME)
            return info.moveTime;

        uint64_t thinkTimeMs = 0;
        uint64_t maxThinkTimeMs = INF_THINK_TIME_MS;

        int movesToGo = info.flags & F_MOVESTOGO
                            ? info.movesToGo + 2
                            : DEFAULT_MOVESTOGO;

        if (info.flags & F_TIME)
        {
            uint64_t timeLeftMs = info.time[side] * (1.0 - SAFETY_MARGIN);
            thinkTimeMs += timeLeftMs / movesToGo;
            maxThinkTimeMs = timeLeftMs / 2;
        }
        if (info.flags & F_INC)
        {
            thinkTimeMs += info.increment[side] * INC_MULTIPLIER;
        }

        thinkTimeMs = std::max(thinkTimeMs, MIN_THINK_TIME_MS);
        thinkTimeMs = std::min(thinkTimeMs, maxThinkTimeMs);

        return thinkTimeMs;
    }

    bool shouldStop(ThinkInfo *info, Depth depth, uint64_t nodes,
                    std::chrono::_V2::steady_clock::time_point endTime)
    {
        if (info == NULL)
            return false;

        if (info->task == NOTHING)
            return true;

        if (info->flags & F_INFINITE)
            return false;
        if (info->flags & F_DEPTH)
            return depth >= info->depth;
        if (info->flags & F_NODES)
            return nodes >= info->nodes;

        return std::chrono::steady_clock::now() >= endTime;
    }
}
