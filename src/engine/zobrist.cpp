#include <random>
#include "zobrist.hpp"

namespace engine
{
    Key pieceTileZ[15][64];
    Key castlingZ[16];
    Key enPassantFileZ[8];
    Key turnZ;

    void zobrist::init()
    {
        std::random_device rnd;
        std::mt19937_64 gen(rnd());
        std::uniform_int_distribution<uint64_t> dist(
            std::llround(std::pow(2, 61)),
            std::llround(std::pow(2, 62)));

        for (int i = 0; i < 15; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                pieceTileZ[i][j] = dist(gen);
            }
        }
        for (int i = 0; i < 16; i++)
        {
            castlingZ[i] = dist(gen);
        }
        for (int i = 0; i < 8; i++)
        {
            enPassantFileZ[i] = dist(gen);
        }
        turnZ = dist(gen);
    }
}
