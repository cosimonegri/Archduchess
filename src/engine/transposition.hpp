#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "evaluation.hpp"
#include "zobrist.hpp"
#include "move.hpp"

namespace engine
{
    struct TTEntry
    {
        Move bestMove;
        Eval eval;
        Depth depth;
    };
}

#endif
