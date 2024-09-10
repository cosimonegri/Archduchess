#include "time.hpp"

namespace engine
{
    uint64_t calcThinkTimeMs(ThinkInfo info, Color side)
    {
        if (info.flags & (F_INFINITE | F_DEPTH | F_NODES))
            return MAX_THINK_TIME_MS;

        if (info.flags & F_MOVETIME)
            return info.moveTime;

        uint64_t thinkTimeMs = 0;
        int movesToGo = info.flags & F_MOVESTOGO
                            ? info.movesToGo
                            : DEFAULT_MOVESTOGO;

        if (info.flags & F_TIME)
            thinkTimeMs += info.time[side] / movesToGo;
        if (info.flags & F_INC)
            thinkTimeMs += info.increment[side];

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
