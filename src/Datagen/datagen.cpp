#include "datagen.h"
#include "../PSQT.h"
#include <fstream>

#ifdef datagen

u64 lastRandom = 1;

void datagenLoop(Position &pos, int gamesToPlay, int timeControl, const std::string& outputFileName, int exitDepth, int reportingFrequency) {
    int playedGames = 0;
    int timeControlInMs = timeControl * 1000;
    std::ofstream outputFile;
    outputFile.open(outputFileName, std::ios::app);

    while (gamesToPlay > playedGames++) {
        gameInfo gi;
        gi.timeW = gi.timeB = timeControlInMs;
        gi.increment = timeControlInMs / 100;

        createExit(pos, gi, exitDepth);

        outputFile << playGame(gi ,pos);

        std::cout << playedGames << "/" << gamesToPlay << " games played\n";
    }

    outputFile.close();
}

bool okExit(Position &pos) {
    SearchInfo si;
    return simpleQ(-1, 1, pos, si) == 0;
}

int pieceEval(Position &pos) {
    int valWhite = 0;
    int valBlack = 0;
    valWhite += __builtin_popcountll(pos.bitBoards[WHITE_PAWN  ]) * PawnValueMG;
    valWhite += __builtin_popcountll(pos.bitBoards[WHITE_KNIGHT]) * KnightValueMG;
    valWhite += __builtin_popcountll(pos.bitBoards[WHITE_BISHOP]) * BishopValueMG;
    valWhite += __builtin_popcountll(pos.bitBoards[WHITE_ROOK  ]) * RookValueMG;
    valWhite += __builtin_popcountll(pos.bitBoards[WHITE_QUEEN ]) * QueenValueMG;

    valBlack += __builtin_popcountll(pos.bitBoards[BLACK_PAWN  ]) * PawnValueMG;
    valBlack += __builtin_popcountll(pos.bitBoards[BLACK_KNIGHT]) * KnightValueMG;
    valBlack += __builtin_popcountll(pos.bitBoards[BLACK_BISHOP]) * BishopValueMG;
    valBlack += __builtin_popcountll(pos.bitBoards[BLACK_ROOK  ]) * RookValueMG;
    valBlack += __builtin_popcountll(pos.bitBoards[BLACK_QUEEN ]) * QueenValueMG;

    return valWhite - valBlack;
}

int simpleQ(int alpha, int beta, Position &pos, SearchInfo &si, int depth) {
    Move currentMove;
    bool check;
    int bestScore = pieceEval(pos);

    if (bestScore >= beta || depth > 16)
        return bestScore;

    Movepicker mp;
    while ((currentMove = pickNextMove<true>(mp, 0, pos, check)) != 0) {
        pos.makeMove(currentMove);
        si.nodeCount++;

        int score = -simpleQ(-beta, -alpha, pos, si, depth + 1);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;

                if (score >= beta)
                    return score;
            }
        }
    }

    return bestScore;
}

void createExit(Position &pos, gameInfo &gi, int exitDepth) {
    int attempts = 0;
    do {
        attempts++;
       pos.setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
       gi.currentPos = "position startpos ";
       gi.fens.clear();
       int depth = 0;

        while (depth++ < exitDepth) {
            MoveList ml;
            int ksq = lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]);
            u64 checkers = attackersTo<false, false>(ksq ,getOccupied<WHITE>(pos) | getOccupied<BLACK>(pos), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
            generateMoves<false>(pos, ml, checkers);

            u64 random = randomULL(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch()).count()) + lastRandom;
            lastRandom = random != 0 ? random : lastRandom;
            int idx = int(random % (ml.length + 1));

            Move move = ml.moves[idx].move;
            if (move == 0)
                continue;
            pos.makeMove(move);
            gi.currentPos += moveToString(ml.moves[idx].move) + " ";
        }
    } while (!okExit(pos));
}

std::string playGame(gameInfo &gi, Position &pos) {
    int score = 0;
    int result = 0;
    std::string resultS;

    std::string game;

    while (true) {
        SearchInfo si;
        si.st = calcThinkingTime(pos.sideToMove ? gi.timeW : gi.timeB, gi.increment);
        (pos.sideToMove ? gi.timeW : gi.timeB) -= int(si.st.thinkingTime.count());
        (pos.sideToMove ? gi.timeW : gi.timeB) += gi.increment;
        score = startSearching(si, pos);

        if (abs(score) > MAXMATE) {
            if (score > 0)
                result = 10 * pos.sideToMove;
            else
                result = 10 * !pos.sideToMove;
            break;
        } else if (si.bestRootMove == 0 || (pos.plys50moveRule > 99)) {
            result = 5;

            if (si.bestRootMove == 0)
                pos.printBoard();
            break;
        }

        gi.fens.push(pos.fen());
        pos.makeMove(si.bestRootMove);
    }
    resultS = result == 0 ? " [0.0]\n" : (result == 10) ? " [1.0]\n" : " [0.5]\n";

    game += "//start\n";
    while (gi.fens.getSize() != 0) {
        game += gi.fens.pop();
        game += resultS;
    }
    game += "//end\n";
    
    return game;
}

int startSearching(SearchInfo &si, Position &pos) {
    int score;
    int lastGoodScore = 0;

    for (int depth = 1; depth != 100; depth++) {
        score = searchRoot(pos, si, depth);

        if (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime)) {
            if (depth == 1)
                lastGoodScore = score;
            break;
        }

        lastGoodScore = score;
    }

    return lastGoodScore;
}

#endif