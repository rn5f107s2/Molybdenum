#include "tuner.h"
#include "../Position.h"
#include "../search.h"
#include <cmath>
#include "../eval.h"
#include "../PSQT.h"
#include "fstream"
#include <iomanip>

#ifdef tuneDef

void tune(Position &pos, const std::string& filename) {
    double smallestError = getErrorAllFens(filename, pos);
    int epochs = 1;
    int changeStage = 32;
    int currentValue = 0;
    bool improving;
    bool decreasing;
    bool increasing;
    bool stageImproving;

    while (changeStage) {
        stageImproving = false;
        for (int i = 0; i != numParams; i++) {
            int bestValue = 0;
            decreasing = false;
            increasing = false;

            do {
                improving = false;
                currentValue = tweakValues(i, changeStage);
                double newError = smallestError;
                if (!decreasing) {
                    newError = getErrorAllFens(filename, pos);
                    std::cout << "Epoch " << ++epochs << "\n";
                }

                if (newError < smallestError) {
                     smallestError = newError;
                     improving = true;
                     increasing = true;
                } else if (!increasing) {
                    currentValue = tweakValues(i, -(changeStage * 2));
                    newError = getErrorAllFens(filename, pos);
                    std::cout << "Epoch " << ++epochs << "\n";

                    if (newError < smallestError) {
                        improving = true;
                        decreasing = true;
                        smallestError = newError;
                    } else {
                        bestValue = tweakValues(i, changeStage);
                    }
                } else {
                    bestValue = tweakValues(i, -changeStage);
                }
                stageImproving = stageImproving || improving;
            } while (improving);

            std::cout << "Tuning " << i << " done\n";
            std::cout << "Value: " << bestValue << "\n";
            std::cout << "Error: " << std::setprecision(15) << smallestError << "\n";
        }

        if (!stageImproving)
            changeStage /= 2;
    }

    printPSQB();
    std::cout << "\n";
    for (int i = 0; i != 6; i++)
        std::cout << PieceValuesMGTune[i] << " ";

    std::cout << "\n";

    for (int i = 0; i != 6; i++)
        std::cout << PieceValuesEGTune[i] << " ";
}

void calcK(Position &pos, const std::string& filename) {
    double smallestError = getErrorAllFens(filename, pos);
    bool improving;
    do {
            improving = false;
            K += 0.01;
            double newError = getErrorAllFens(filename, pos);

            if (newError < smallestError) {
                smallestError = newError;
                improving = true;
            }else {
                K -= 0.02;
                newError = getErrorAllFens(filename, pos);

                if (newError < smallestError) {
                    improving = true;
                    smallestError = newError;
                }
            }
        std::cout << "Error: " << std::setprecision(15) << smallestError << "\n";
    } while (improving);

    K += 0.01;
    std::cout << "Optimal K: " << std::setprecision(15) << K << "\n";
}

int tuneQ(int alpha, int beta, Position &pos) {
    Move currentMove;
    bool check;
    int bestScore = tuneEval(pos);

    if (bestScore >= beta)
        return bestScore;

    alpha = std::max(bestScore, alpha);

    Movepicker mp;
    while ((currentMove = pickNextMove<true>(mp, 0, pos, check)) != 0) {
        pos.makeMove(currentMove);

        int score = -tuneQ(-beta, -alpha, pos);

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

void printPSQB() {
    for (int i = 0; i != 2; i++) {
        for (int j = 0; j != 6; j++) {
            std::cout << "Start piece " << j << "\n";

            for (int k = 0; k != 64; k++) {
                int value = PieceSquareBonusesTune[i][j][k];
                std::string toPrint;
                int length = int(std::to_string(value).size());

                while (length++ < 4)
                    toPrint += " ";

                toPrint += std::to_string(value);

                std::cout << toPrint << ", ";

                if (k % 8 == 7)
                    std::cout << "\n";
            }
        }
        std::cout << "starteg\n";
    }
}

int tweakValues(int index, int amount) {
    if (index < 768) {
        int pcidx = (index % 384) / 64;
        return PieceSquareBonusesTune[index < 384][pcidx][index % 64] += amount;
    } else if (index < 774) {
        return PieceValuesMGTune[index % 6] += amount;
    } else
        return PieceValuesEGTune[index % 6] += amount;
}

double getErrorAllFens(const std::string& filename, Position &pos) {
    double error = 0;
    int amountOfFens = 0;
    std::string fen;
    std::ifstream data(filename);

    if (!data) {
        std::cerr << "couldnt open file\n";
        return 0;
    }

    while (getline(data, fen)) {
        if ((fen == "//start") || (fen == "//end"))
            continue;

        amountOfFens++;
        error += getErrorFen(fen, pos);
    }
    data.close();

    return error / amountOfFens;
}

int tuneEval(Position &pos) {
    const int maxPhaseTune =  gamePhaseValues[KNIGHT] * 4
                            + gamePhaseValues[BISHOP] * 4
                            + gamePhaseValues[ROOK  ] * 4
                            + gamePhaseValues[QUEEN ] * 2;

    int phase = 0;
    phase += __builtin_popcountll(pos.bitBoards[WHITE_KNIGHT] | pos.bitBoards[BLACK_KNIGHT]) * gamePhaseValuesTune[KNIGHT];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_BISHOP] | pos.bitBoards[BLACK_BISHOP]) * gamePhaseValuesTune[BISHOP];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_ROOK  ] | pos.bitBoards[BLACK_ROOK  ]) * gamePhaseValuesTune[ROOK  ];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_QUEEN ] | pos.bitBoards[BLACK_QUEEN ]) * gamePhaseValuesTune[QUEEN ];
    phase = std::min(phase, maxPhaseTune);

    int mgEval = 0;
    int egEval = 0;

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = pos.bitBoards[pc];
        int pt = typeOf(pc);

        while (pieceBB) {
            int square = popLSB(pieceBB);
            int sq = square ^ (pc < BLACK_PAWN ? 63 : 7);

            mgEval += (PieceSquareBonusesTune[0][pt][sq] + PieceValuesMGTune[pt]) * ((pc >= BLACK_PAWN) ? -1 : 1);
            egEval += (PieceSquareBonusesTune[1][pt][sq] + PieceValuesEGTune[pt]) * ((pc >= BLACK_PAWN) ? -1 : 1);
        }
    }

    int eval = (mgEval * phase + egEval * (maxPhase - phase)) / maxPhase;
    return pos.sideToMove ? eval : -eval;
}

double getWinProbability(double score) {
    return double(1) / (1 + pow(10, -K * (score / 400)));
}

double getMeanSquaredError(int score, double correct) {
    double error = correct - getWinProbability(double(score));
    error *= error;
    return error;
}

double getErrorFen(const std::string& fen, Position &pos) {
    double result = std::stod(fen.substr(fen.length() - 4, 3));
    pos.setBoard(fen.substr(0 ,fen.length() - 6));
    int score = tuneEval(pos) * (pos.sideToMove ? 1 : -1);
    return getMeanSquaredError(score, result);
}

#endif