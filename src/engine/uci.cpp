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
    void UCIEngine::loop()
    {
        std::string command, token;

        while (true)
        {
            if (!std::getline(std::cin, command))
                command = "quit";

            // log(command);
            std::istringstream iss(command);
            token.clear();
            iss >> token;

            if (token == "uci")
                respond("uciok");

            else if (token == "isready")
                respond("readyok");

            else if (token == "ucinewgame")
                continue;

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
                int depth = std::stoi(token);
                runPerft(depth);
            }
            catch (...)
            {
            }
        }
        else
        {
            // todo engine should be able to read input while thinking
            respond("bestmove " + bot.chooseMove());
            return;
        }
    }

    void UCIEngine::printPosition()
    {
        bot.getPosition().print();
    }

    void UCIEngine::runPerft(int depth)
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
