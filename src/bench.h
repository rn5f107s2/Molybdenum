#ifndef MOLYBDENUM_BENCH_H
#define MOLYBDENUM_BENCH_H

#include <array>
#include <string>

const int BENCH_DEPTH = 7;
const int BENCH_SIZE = 15;

std::array<std::string, BENCH_SIZE> positions =
        {
            "8/8/4kpp1/3p1b2/p6P/2B5/6P1/6K1 b - - 2 47", // https://www.chessgames.com/perl/chessgame?gid=1143956, Bh3!
            "1r4nk/1p1qb2p/3p1r2/p1pPp3/2P1Pp2/5P1P/PP1QNBRK/5R2 b - - 3 30", // https://www.chessgames.com/perl/chessgame?gid=1084375, Qxh3
            "4r3/1k3p1p/2pr4/2Bn4/PP6/3B1pP1/R3p2P/4R1K1 b - - 0 33", // Nf4 ~=
            "r4rk1/pp3pbp/1qp3p1/2B5/2BP2b1/Q1n2N2/P4PPP/3RK2R b K - 1 16", // https://www.chessgames.com/perl/chessgame?gid=1008361 Be6
            "2r2rk1/1bpR1p2/1pq1pQp1/p3P2p/P1PR3P/5N2/2P2PPK/8 w - - 2 32", // https://www.chessgames.com/perl/chessgame?gid=1124533 Kg3
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // startpos
            "r1b1k2r/1p2bpp1/1qn1p3/p1ppPn2/5P1p/1P1P1N1P/PBPQN1P1/1K1R1B1R b kq - 1 13",
            "r1bq1rk1/pp2ppbp/2np1np1/8/3NP3/2N1BP2/PPPQ2PP/2KR1B1R b - - 4 9", // d5
            "8/p4p2/5pkp/1pr5/2P1KP2/6P1/P1R4P/8 b - - 1 32", // Rxc4 0-1
            "1rqb1rk1/3b1ppp/3p4/1p1Np3/p3P3/P1PQ4/1P2BPPP/3R1RK1 w - - 6 22",
            "4r1k1/pp4p1/5p2/3p4/q1pP1R1P/2P1r2P/1P3Q1K/6R1 w - - 0 1", // Rfg4 ~=
            "2r3k1/p1n1pp1p/b5p1/q1pPp3/2P4P/NR2Q3/P2N1PP1/6K1 b - - 0 24", // Ne8 ~= 
            "2rq1r2/pp1bppbk/3p2p1/5P1n/3NP3/1BN1n3/PPPQ3P/2KR2R1 w - - 0 17", // Bxf7 ~ +1
            "2rq1r2/pp1b2k1/3pppPb/6Rn/3NP2P/1BN1Q3/PPP5/2KR4 w - - 1 21", // Nf3 ~ -1
            "8/8/p2rk2p/3p1p1q/PPpBpR2/2P1K1P1/7P/R7 w - - 1 35"
        };

#endif //MOLYBDENUM_BENCH_H
