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
                bot.startNewGame();

            else if (token == "position")
                processPosition(iss);

            else if (token == "go")
                processGo(iss);

            else if (token == "stop")
                bot.stopThinking();

            else if (token == "quit")
            {
                bot.stopThinking();
                exit(0);
            }

            else if (token == "d")
                printPosition();
        }
    }

    void UCIEngine::respond(std::string message)
    {
        std::cout << message << std::endl;
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
            Depth depth = readNextInt(iss);
            runPerft(depth);
        }
        else
        {
            ThinkInfo info;
            do
            {
                readGoParameters(info, iss, token);
            } while (iss >> token);
            if (info.flags == NO_THINK_FLAG)
            {
                info.flags |= F_INFINITE;
            }
            bot.startThinking(info);
        }
    }

    void UCIEngine::readGoParameters(ThinkInfo &info, std::istringstream &iss, std::string &token)
    {
        if (token == "searchmoves")
        {
            // not implemented
            info.flags |= F_INFINITE;
        }
        else if (token == "ponder")
        {
            // not implemented
            info.flags |= F_INFINITE;
        }
        else if (token == "wtime")
        {
            info.flags |= F_TIME;
            info.time[WHITE] = readNextInt(iss);
        }
        else if (token == "btime")
        {
            info.flags |= F_TIME;
            info.time[BLACK] = readNextInt(iss);
        }
        else if (token == "winc")
        {
            info.flags |= F_INC;
            info.increment[WHITE] = readNextInt(iss);
        }
        else if (token == "binc")
        {
            info.flags |= F_INC;
            info.increment[BLACK] = readNextInt(iss);
        }
        else if (token == "movestogo")
        {
            info.flags |= F_MOVESTOGO;
            info.movesToGo = readNextInt(iss);
        }
        else if (token == "depth")
        {
            info.flags |= F_DEPTH;
            info.depth = readNextInt(iss);
        }
        else if (token == "nodes")
        {
            info.flags |= F_NODES;
            info.nodes = readNextInt(iss);
        }
        else if (token == "mate")
        {
            // not implemented
            info.flags |= F_INFINITE;
        }
        else if (token == "movetime")
        {
            info.flags |= F_MOVETIME;
            info.moveTime = readNextInt(iss);
        }
        else if (token == "infinite")
        {
            info.flags |= F_INFINITE;
        }
    }

    int UCIEngine::readNextInt(std::istringstream &iss)
    {
        std::string token;
        iss >> token;
        return std::stoi(token);
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
