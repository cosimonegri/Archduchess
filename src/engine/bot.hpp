#ifndef BOT
#define BOT

#include <string>
#include <map>
#include <thread>
#include <semaphore>
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

    constexpr int MAX_THINK_TIME_MS = 100;

    class Bot : public SearchListener
    {
    private:
        SearchManager SM;
        Position pos;

        MoveListener *listener;

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
        void startThinking();
        void stopThinking();
        void onSearchComplete(Move move) override;
    };
}

#endif
