#include <string>
#include <iostream>
#include "UCI.h"
#include "Position.h"
#include "BitStuff.h"
#include "Perft.h"
#include "search.h"
#include <chrono>
#include "eval.h"

void uciCommunication() {
    Position internalBoard = Position();
    std::string input;

    while (true) {
        std::getline(std::cin, input);

        if (contains(input, "quit"))
            return;

        if (contains(input, "uci"))
            std::cout << "id name Molybdenum\nid author rn5f107s2\nuciok\n";

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
            std::cout << "Speed: " << (nodeCount / (milliseconds + 1)) / 1000 << " mnps";
            continue;
        }

        if (contains(input, "go")) {
            startSearch(internalBoard);
            continue;
        }

        if (contains(input, "eval")) {
            std::cout << evaluate(internalBoard) << "\n";
        }
    }
}
