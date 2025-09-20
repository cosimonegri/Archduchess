#ifndef BOT
#define BOT

#include <string>
#include <map>
#include <thread>
#include <semaphore>
#include <atomic>
#include "search.hpp"
#include "listeners.hpp"
#include "position.hpp"
#include "move.hpp"

namespace engine
{
    static const std::map<MoveFlag, char> PROM_TO_CHAR = {
        {KNIGHT_PROM, 'n'},
        {KNIGHT_PROM_CAPTURE, 'n'},
        {BISHOP_PROM, 'b'},
        {BISHOP_PROM_CAPTURE, 'b'},
        {ROOK_PROM, 'r'},
        {ROOK_PROM_CAPTURE, 'r'},
        {QUEEN_PROM, 'q'},
        {QUEEN_PROM_CAPTURE, 'q'},
    };

    std::string moveToUci(Move move);
    std::string moveToSan(Position &pos, Move move);

    class Bot : public SearchListener
    {
    private:
        Position pos;
        SearchManager SM;

        MoveListener *listener;

        ThinkInfo thinkInfo;
        std::thread thinkThread;
        std::binary_semaphore thinkSemaphore;

        void runThinkThread();

    public:
        Bot();
        ~Bot();

        Position getPosition();
        void setPosition(std::string fen);
        void makeTurn(std::string move);
        void setListener(MoveListener *listener);

        void startNewGame();
        void startThinking(ThinkInfo info);
        void stopThinking();
        void onSearchInfo(Depth depth, uint64_t nodes, uint64_t timeMs,
                          float ttOccupancy) override;
        void onSearchComplete(Move move) override;
    };
}

#endif
