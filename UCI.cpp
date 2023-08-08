#include <string>
#include <iostream>
#include "UCI.h"
#include "Position.h"
#include "BitStuff.h"

void uciCommunication() {
    Position internalBoard = Position();
    std::string input;

    while (true) {
        std::getline(std::cin, input);

        if (contains(input, "quit"))
            return;

        if (contains(input, "position")) {
            std::string fen;

            if (contains(input, "startpos"))
                fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            else
                fen = extractFEN(input);
            internalBoard.setBoard(fen);

            if (contains(input, "moves")) {
                int loopCount;
                size_t start = input.find("moves") + 6;
                std::string moves = input.substr(start, input.length());

                while (!moves.empty()) {
                    int promotionPiece = PROMO_KNIGHT;
                    int fromSquare;
                    int toSquare;
                    std::string move = moves.substr(0, moves.find(' '));

                    if (move.length() == 5)
                        promotionPiece = charIntToPiece(toupper(move.at(4))) - 1;

                    u64 from = stringToSquare(move.substr(0, 2));
                    u64 to   = stringToSquare(move.substr(2, 2));

                    fromSquare = lsb(from);
                    toSquare   = lsb(to);

                    internalBoard.makeMove(internalBoard.fromToToMove(fromSquare, toSquare, promotionPiece));

                    if (moves.find(' ') == std::string ::npos)
                        break;
                    moves = moves.substr(moves.find(' '));
                }
            }
        }

        if (input == "d")
            internalBoard.printBoard();
    }
}
