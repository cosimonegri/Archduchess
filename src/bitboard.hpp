#ifndef BITBOARD
#define BITBOARD

#include <vector>
#include <cassert>
#include "types.hpp"

namespace engine
{
    namespace bitboard
    {
        void init();
        void print(Bitboard b);
        void generateBlockers(Bitboard movementMask, std::vector<Bitboard> &blockers);
    }

    constexpr Bitboard fileA = 0x0101010101010101ULL;
    constexpr Bitboard fileB = fileA << 1;
    constexpr Bitboard fileC = fileA << 2;
    constexpr Bitboard fileD = fileA << 3;
    constexpr Bitboard fileE = fileA << 4;
    constexpr Bitboard fileF = fileA << 5;
    constexpr Bitboard fileG = fileA << 6;
    constexpr Bitboard fileH = fileA << 7;

    constexpr Bitboard rank1 = 0xffULL;
    constexpr Bitboard rank2 = rank1 << 8;
    constexpr Bitboard rank3 = rank1 << 16;
    constexpr Bitboard rank4 = rank1 << 24;
    constexpr Bitboard rank5 = rank1 << 32;
    constexpr Bitboard rank6 = rank1 << 40;
    constexpr Bitboard rank7 = rank1 << 48;
    constexpr Bitboard rank8 = rank1 << 56;

    extern Bitboard betweenBB[64][64];
    extern int manhattanDistance[64][64];

    extern Bitboard pawnAttacks[2][64];
    extern Bitboard pseudoAttacks[7][64];

    // the size depends on the worst (lowest) magic shift
    extern Bitboard rookMagics[64][16384];
    extern Bitboard bishopMagics[64][2048];

    // these have been computed using a function in utils.cpp
    static const Bitboard ROOK_MAGIC_NUMBERS[64] = {14168668547763915772ULL, 3875625291025709983ULL, 5305752309726181668ULL, 8733511598333400245ULL, 11443603976976405858ULL, 18082796148885292900ULL, 13492861972479372005ULL, 16885870079474472360ULL, 1474293741318405145ULL, 3431890785133015648ULL, 13767795805764996565ULL, 15344422516827720041ULL, 17115561066144999560ULL, 17588146049744549041ULL, 16910400880976105729ULL, 17798601266626014022ULL, 8212195428985369068ULL, 550342953619558257ULL, 5166051718067416577ULL, 13058629318816845267ULL, 883089539731537408ULL, 10712098521701298499ULL, 5691216180035026414ULL, 15689077511333532371ULL, 45894779818419972ULL, 7993375556532614071ULL, 14669590865791011753ULL, 7162567453375249422ULL, 17102975931187384740ULL, 13964066156076506338ULL, 14772254610365216929ULL, 3776049527499379146ULL, 4401812177490590356ULL, 7257891170587440261ULL, 1598734132217305412ULL, 1841595127433507810ULL, 2252476785093064864ULL, 10198992735187805428ULL, 6006412929614733561ULL, 991383092850147378ULL, 7438296837575941801ULL, 12112295410144518381ULL, 12860194086864470549ULL, 13372877821791774295ULL, 1070076172497951345ULL, 1171627085530314133ULL, 2771912069148067222ULL, 1336538562976055311ULL, 11301854736696869440ULL, 13216613213490985587ULL, 5124508682018912436ULL, 15501490743484255658ULL, 17656472668214221940ULL, 10551659931469556781ULL, 12978827885268405646ULL, 2395658910856666256ULL, 9613276411756708054ULL, 1475703634037179350ULL, 878764786655396210ULL, 821904729637434022ULL, 11338509043517093906ULL, 9645530325025745019ULL, 13973162475916342436ULL, 5016470431788696810ULL};
    static const unsigned ROOK_MAGIC_SHIFTS[64] = {50, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 53, 53, 51, 51, 53, 53, 52, 52, 52, 53, 52, 51, 53, 52, 52, 52, 52, 53, 52, 52, 53, 52, 52, 52, 52, 53, 52, 51, 52, 53, 52, 52, 52, 53, 52, 52, 53, 52, 52, 53, 52, 53, 52, 51, 52, 52, 52, 51, 51, 52, 52};
    static const Bitboard BISHOP_MAGIC_NUMBERS[64] = {12022524710329687762ULL, 8707736504044751749ULL, 7705672187898242757ULL, 16384271926834133526ULL, 3010735797082754920ULL, 15572606522767167279ULL, 505541677072577670ULL, 8707836444909753072ULL, 5256616179373352330ULL, 7490031656062314329ULL, 8360589813208452612ULL, 6766240849187202710ULL, 8732299444010234260ULL, 8095811696112629010ULL, 4018808582698320005ULL, 13449369775277662223ULL, 11822358432366734634ULL, 5549147248328908844ULL, 2858110899231705820ULL, 1229422534382863012ULL, 57288320873159713ULL, 13951527793579540764ULL, 16163899254536590477ULL, 16276327954655545425ULL, 17629795105785352006ULL, 3352969552377100329ULL, 17754068931298072128ULL, 17225073970085300614ULL, 11981984493624594382ULL, 11898552091916921149ULL, 17712426365282880252ULL, 18373282082841288060ULL, 12206616670598455353ULL, 11407281989990178953ULL, 13340574951256067431ULL, 11695874448775840253ULL, 4439695318095068895ULL, 2780665326407277486ULL, 18095611522545930428ULL, 13514758850565513757ULL, 17810613979390066909ULL, 9804069767419301909ULL, 11702511274606823529ULL, 17058947219799154464ULL, 11787157313989653596ULL, 15524696669592251090ULL, 1918579733512099345ULL, 12533564073731921157ULL, 17540196805672711869ULL, 16482897587364697438ULL, 15162500680135483618ULL, 9118140069798484473ULL, 12124853756487811286ULL, 14297275684270342519ULL, 14242039724707679139ULL, 15651174410979255710ULL, 187280100218578412ULL, 1378614946686541978ULL, 1535217175005450260ULL, 4680794541475891724ULL, 8999181190618459146ULL, 14865814990856129031ULL, 9270765750243035924ULL, 17344353467731415080ULL};
    static const unsigned BISHOP_MAGIC_SHIFTS[64] = {57, 59, 59, 59, 59, 59, 59, 57, 59, 59, 59, 59, 59, 59, 59, 59, 59, 58, 56, 56, 56, 56, 58, 59, 58, 59, 56, 53, 53, 56, 58, 58, 59, 58, 56, 53, 53, 56, 58, 59, 59, 59, 56, 56, 56, 56, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 57, 59, 59, 59, 59, 59, 59, 57};

    inline unsigned rookMagicKey(Tile from, Bitboard occupied)
    {
        Bitboard blockers = occupied & pseudoAttacks[ROOK][from];
        return (blockers * ROOK_MAGIC_NUMBERS[from]) >> ROOK_MAGIC_SHIFTS[from];
    };

    inline unsigned bishopMagicKey(Tile from, Bitboard occupied)
    {
        Bitboard blockers = occupied & pseudoAttacks[BISHOP][from];
        return (blockers * BISHOP_MAGIC_NUMBERS[from]) >> BISHOP_MAGIC_SHIFTS[from];
    };

    inline Bitboard tileBB(Tile tile)
    {
        assert(isValid(tile));
        return (1ULL << tile);
    }

    constexpr Bitboard fileBB(File file)
    {
        return fileA << file;
    }

    constexpr Bitboard rankBB(Rank rank)
    {
        return rank1 << (8 * rank);
    }

    inline void setBit(Bitboard &bitboard, Tile tile)
    {
        bitboard |= tileBB(tile);
    }

    inline void clearBit(Bitboard &bitboard, Tile tile)
    {
        bitboard &= ~tileBB(tile);
    }

    template <Direction D>
    constexpr Bitboard shiftBB(Bitboard b)
    {
        return D == UP           ? b << 8
               : D == DOWN       ? b >> 8
               : D == RIGHT      ? (b & ~fileH) << 1
               : D == LEFT       ? (b & ~fileA) >> 1
               : D == UP_RIGHT   ? (b & ~fileH) << 9
               : D == UP_LEFT    ? (b & ~fileA) << 7
               : D == DOWN_RIGHT ? (b & ~fileH) >> 7
               : D == DOWN_LEFT  ? (b & ~fileA) >> 9
                                 : 0;
    }

    inline Bitboard getBetweenBB(Tile tileA, Tile tileB)
    {
        assert(isValid(tileA) && isValid(tileB));
        return betweenBB[tileA][tileB];
    }

    template <PieceType PT>
    inline Bitboard getAttacksBB(Tile from, Bitboard occupied = 0ULL)
    {
        assert(PT != PAWN && isValid(from));
        switch (PT)
        {
        case KNIGHT:
            return pseudoAttacks[KNIGHT][from];
        case BISHOP:
            return bishopMagics[from][bishopMagicKey(from, occupied)];
        case ROOK:
            return rookMagics[from][rookMagicKey(from, occupied)];
        case QUEEN:
            return getAttacksBB<BISHOP>(from, occupied) | getAttacksBB<ROOK>(from, occupied);
        case KING:
            return pseudoAttacks[KING][from];
        default:
            return 0;
        }
    }

    inline Bitboard getAttacksBB(PieceType pt, Tile from, Bitboard occupied = 0ULL)
    {
        assert(pt != PAWN && isValid(from));
        switch (pt)
        {
        case KNIGHT:
            return getAttacksBB<KNIGHT>(from, occupied);
        case BISHOP:
            return getAttacksBB<BISHOP>(from, occupied);
        case ROOK:
            return getAttacksBB<ROOK>(from, occupied);
        case QUEEN:
            return getAttacksBB<BISHOP>(from, occupied) | getAttacksBB<ROOK>(from, occupied);
        case KING:
            return getAttacksBB<KING>(from, occupied);
        default:
            return 0;
        }
    }

    // Return the least significant 1 bit of a non-zero bitboard
    inline Tile lsb(Bitboard bitboard)
    {
        assert(bitboard != 0);
        return Tile(__builtin_ctzll(bitboard));
    }

    // Return and pop the least significant 1 bit of a non-zero bitboard
    inline Tile popLsb(Bitboard &bitboard)
    {
        Tile tile = lsb(bitboard);
        bitboard &= bitboard - 1;
        return tile;
    }
}

#endif
