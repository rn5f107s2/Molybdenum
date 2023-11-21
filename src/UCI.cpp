#include <string>
#include <iostream>
#include "UCI.h"
#include "Position.h"
#include "BitStuff.h"
#include "Perft.h"
#include "search.h"
#include <chrono>
#include "eval.h"
#include "timemanagement.h"
#include "UCIOptions.h"
#include "searchUtil.h"
#include "Datagen/Datagen.h"

UCIOptions options;

const std::string name = "Molybdenum";
const std::string version = "1.0";

void uciCommunication() {
    Position internalBoard;

#ifdef DATAGEN
    std::string filename;
    std::cin >> filename;
    start(internalBoard, filename);
#endif

    internalBoard.setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::string input;
    options.init();

    while (true) {
        std::getline(std::cin, input);

        if (contains(input, "quit"))
            return;

        if (contains(input, "ucinewgame")) {
            clearHistory();
            continue;
        }

        if (contains(input, "uci")) {
            std::cout << "id name " << name << " " << version << "\n";
            std::cout << "id author rn5f107s2\n";
            options.printOptions();
            std::cout << "uciok\n";
            continue;
        }

        if (contains(input, "isready"))
            std::cout << "readyok\n";


        if (contains(input, "position")) {
            std::string fen;

            if (contains(input, "startpos"))
                fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            else
                fen = extractFEN(input);
            internalBoard.setBoard(fen);

            if (contains(input, "moves")) {
                size_t start = input.find("moves") + 6;
                std::string moves = input.substr(start, input.length());

                while (!moves.empty()) {
                    int promotionPiece = PROMO_KNIGHT;
                    int fromSquare;
                    int toSquare;
                    int flag = NORMAL;
                    std::string move = moves.substr(0, moves.find(' '));

                    if (move.length() == 5) {
                        promotionPiece = charIntToPiece(toupper(move.at(4)) - '0') - 1;
                        flag = PROMOTION;
                    }

                    u64 from = stringToSquare(move.substr(0, 2));
                    u64 to   = stringToSquare(move.substr(2, 2));

                    fromSquare = lsb(from);
                    toSquare   = lsb(to);

                    internalBoard.makeMove(internalBoard.fromToToMove(fromSquare, toSquare, promotionPiece, flag));

                    if (moves.find(' ') == std::string ::npos)
                        break;
                    moves = moves.substr(moves.find(' ') + 1);
                }
            }
        }

        if (input == "d") {
            internalBoard.printBoard();
            continue;
        }

        if (contains(input, "go perft")) {
            std::string depthS = input.substr(9);
            int depthI = std::stoi(depthS);

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            u64 nodeCount = startPerft(depthI, internalBoard, true);

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();

            std::cout << "______________\n";
            std::cout << "Nodes searched: " << nodeCount << "\n";
            std::cout << "Took: " << milliseconds << " milliseconds" << "\n";
            std::cout << "Speed: " << (nodeCount / (milliseconds + 1)) / 1000 << " mnps\n";
            continue;
        }

        if (contains(input, "go")) {
            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;
            int depth = MAXDEPTH;

            if (contains(input, "wtime")) {
                int start = int(input.find("wtime")) + 6;
                int end   = int(input.find(' ', start));

                wtime = std::stoi(input.substr(start, end));

                start = int(input.find("btime")) + 6;
                end   = int(input.find(' ', start));

                btime = std::stoi(input.substr(start, end));

                if (contains(input, "winc")) {
                    start = int(input.find("winc")) + 5;
                    end   = int(input.find(' ', start));

                    winc = std::stoi(input.substr(start, end));

                    start = int(input.find("binc")) + 5;
                    end   = int(input.find(' ', start));

                    binc = std::stoi(input.substr(start, end));
                }
            }

            int timeLeft  = internalBoard.sideToMove ? wtime : btime;
            int increment = internalBoard.sideToMove ? winc  : binc;

            searchTime st = calcThinkingTime(timeLeft, increment);

            if (contains(input, "nodes ")) {
                int nodes = std::stoi(input.substr(input.find("nodes ") + 6));
                st.nodeLimit = nodes;
                st.limit = Nodes;
            }

            if (contains(input, "depth "))
                depth = std::stoi(input.substr(input.find("depth ") + 6)) + 1;

            if (contains(input, "depth") || contains(input, "infinite"))
                st.limit = Depth;

            startSearch(internalBoard, st, depth);
            continue;
        }

        if (contains(input, "eval")) {
            std::cout << evaluate(internalBoard) << "\n";
        }

        if (contains(input, "setoption")) {
            std::string optionName = input.substr(input.find("name ") + 5);
            int nameEnd = int(optionName.find("value "));
            int value = std::stoi(optionName.substr(nameEnd + 6));
            optionName = optionName.substr(0, optionName.size() - optionName.substr(nameEnd).size() - 1);
            options.setOption(optionName, value);
        }
    }
}
