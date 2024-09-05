#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdint>
#include <cassert>
#include "uci.hpp"
#include "position.hpp"
#include "perft.hpp"
#include "misc.hpp"

namespace engine
{
    UCIEngine::UCIEngine() : bot{Bot()}
    {
        bot.setListener(this);
    }

    void UCIEngine::loop()
    {
        std::string command, token;

        while (true)
        {
            if (!std::getline(std::cin, command))
                command = "quit";

            std::istringstream iss(command);
            token.clear();
            iss >> token;

            if (token == "uci")
                respond("uciok");

            else if (token == "isready")
                respond("readyok");

            else if (token == "ucinewgame")
            {
                bot.startNewGame();
                continue;
            }

            else if (token == "position")
                processPosition(iss);

            else if (token == "go")
                processGo(iss);

            else if (token == "stop")
                continue;

            else if (token == "quit")
                break;

            else if (token == "d")
                printPosition();
        }
    }

    void UCIEngine::respond(std::string message)
    {
        std::cout << message << std::endl;
        // log(message);
    }

    void UCIEngine::processPosition(std::istringstream &iss)
    {
        std::string token, fen;
        iss >> token;

        if (token == "startpos")
        {
            fen = START_FEN;
            iss >> token;
        }
        else if (token == "fen")
        {
            while (iss >> token && token != "moves")
            {
                fen += token + " ";
            }
        }
        else
        {
            return;
        }

        bot.setPosition(fen);
        while (iss >> token)
        {
            bot.makeTurn(token);
        }
    }

    void UCIEngine::processGo(std::istringstream &iss)
    {
        std::string token;
        iss >> token;

        if (token == "perft")
        {
            iss >> token;
            try
            {
                Depth depth = std::stoi(token);
                runPerft(depth);
            }
            catch (...)
            {
            }
        }
        else
        {
            bot.startThinking();
        }
    }

    void UCIEngine::onReceiveInfo(Depth depth, uint64_t nodes, uint64_t timeMs, float ttOccupancy)
    {
        timeMs = std::max(1ull, timeMs);
        uint64_t nps = nodes / timeMs * 1000;
        int hashfull = ttOccupancy * 1000;
        respond(std::vformat("info depth {} nodes {} nps {} hashfull {} time {} ",
                             std::make_format_args(depth, nodes, nps, hashfull, timeMs)));
    }

    void UCIEngine::onMoveChosen(std::string move)
    {
        respond("bestmove " + move);
    }

    void UCIEngine::printPosition()
    {
        bot.getPosition().print();
    }

    void UCIEngine::runPerft(Depth depth)
    {
        Position pos = bot.getPosition();
        auto begin = std::chrono::steady_clock::now();
        uint64_t nodes = perft(pos, depth);
        auto end = std::chrono::steady_clock::now();
        int64_t elapsedTime = getTimeMs(begin, end);

        std::cout << "\nNodes:\t" << nodes << std::endl;
        std::cout << "Time:\t" << elapsedTime << " ms" << std::endl;
        std::cout << "NPS:\t" << (nodes / elapsedTime) << "k" << std::endl
                  << std::endl;
    }
}
