#pragma once

#include <fstream>
#include <vector> 
#include <algorithm>

#include "../UCI.h"
#include "util/viriformat.h"

struct Entry {
    std::string fen;
    int16_t score;
    int result; // 2 = White win, 1 = Draw, 0 = Black win
    std::string move;
};

static int skipped = 0;

Entry parseNextGame(std::ifstream& in, std::vector<Entry>& entries);
Entry parseNextUnfilteredGame(std::ifstream& in, std::vector<Entry>& entries);
void processGame(std::vector<Entry>& entries, std::ofstream& out);
void processUnfilteredGame(std::vector<Entry>& entries, std::ofstream& out);
bool findPos(Position& pos, Position& target, int depthDiff, std::vector<Move>& moves, bool first = false);
Move moveFromUci(Position& pos, std::string move);

void unfilteredLegacyToVF(std::ifstream& in, std::ofstream& out) {
    std::vector<Entry> entries;

    int count = 0;
    int positions = 0;

    // throw away first game in case of bad split
    //parseNextUnfilteredGame(in, entries);
    entries.clear();

    while (!in.eof()) {
        Entry next = parseNextUnfilteredGame(in, entries);

        processUnfilteredGame(entries, out);

        positions += entries.size();

        if (++count % 1000 == 0)
            std::cout << count << " games with " << positions << " Positions done" << std::endl;

        entries.clear();
        entries.push_back(next);
    }
}

void filteredLegacyToVF(std::ifstream& in, std::ofstream& out) {
    std::vector<Entry> entries;

    int count = 0;
    int positions = 0;

    // throw away first game in case of bad split
    parseNextGame(in, entries);
    entries.clear();

    while (!in.eof()) {
        Entry next = parseNextGame(in, entries);

        processGame(entries, out);

        positions += entries.size();

        if (++count % 1000 == 0)
            std::cout << count << " games with " << positions << " Positions done" << std::endl;

        entries.clear();
        entries.push_back(next);
    }
}

inline bool isLegal(Position& pos, Move m) {
    MoveList ml;
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    generateMoves<false>(pos, ml, checkers);

    bool found = false;

    for (int i = 0; i < ml.length; i++) {
        found |= ml.moves[i].move == m;
    }

    return found;
}

inline void pgnToVF(std::ifstream& in, std::ofstream& out) {
    Position pos;
    std::string line;

    while (!in.eof()) {
        Entry entry;

        while (std::getline(in, line), !line.empty()) {
            if (line.substr(1, 3) == "FEN") {
                entry.fen = line.substr(6, line.size() - 8);
            }

            if (line.substr(1, 6) == "Result") {
                std::string res = line.substr(9, line.size() - 11);
                entry.result = res == "1-0" ? 2 : res == "0-1" ? 0 : 1;
            }
        }

        if (in.eof())
            break;

        if (entry.fen.empty()) {
            std::cout << "Did not find FEN in header!" << std::endl;
            exit(-1);
        }

        pos.setBoard(entry.fen);

        ViriFormatGame game(MarlinFormatPackedBoard(pos, entry.result));

        std::getline(in, line);
        std::vector<std::string> moveComments = split(line);

        for (auto it = moveComments.begin(); it < moveComments.end() - 1; it += 2) {
            std::string move    = *it;
            std::string comment = *(it + 1);

            Move m = pos.sanToMove(move);
            std::string s = split(comment.substr(1, comment.size() - 2), '/')[0];
            int score = 0;

            if (!isLegal(pos, m)) {
                std::cout << "Did not find move " << moveToString(m) << " with SAN " << move << "!" << std::endl;
                pos.printBoard();
                exit(-1);
            }

            if (s == "unknown" || contains(s, "M"))
                score = 32767 * (pos.sideToMove == WHITE ? 1 : -1);
            else
                score = int(std::stod(s) * 100) * (pos.sideToMove == WHITE ? 1 : -1);

            game.moves.push_back(ViriFormatMoveEntry(m, score));

            pos.makeMove(m);
        }

        std::getline(in, line);

        out << game;
    }

    delete pos.net;
}

void processGame(std::vector<Entry>& entries, std::ofstream& out) {
    Position pos;
    std::vector<Move> moves;

    pos.setBoard(entries.rbegin()->fen);

    ViriFormatGame game(MarlinFormatPackedBoard(pos, entries.rbegin()->result));

    int16_t score = entries.rbegin()->score;

    for (auto it = entries.rbegin() + 1; it < entries.rend(); ++it) {
        Position target(pos.net); target.setBoard(it->fen);
        moves.clear();

        int entryMc   = std::stoi(split(it->fen, ' ')[5]);
        int depthDiff = entryMc - pos.movecount;

        if (depthDiff >= 9 || depthDiff < 0) {
            std::cout << "Skipped " << skipped++ << std::endl;
            return;
        }

        bool found = findPos(pos, target, depthDiff, moves, true);

        if (!found) {
            std::cout << "Did NOT find pos " << it->fen << " from " << pos.fen() << " ! quitting." << std::endl; 
            exit(-1);
        }

        game.moves.push_back(ViriFormatMoveEntry(moves.front(), score));

        for (auto it = moves.begin() + 1; it < moves.end(); it++)
            game.moves.push_back(ViriFormatMoveEntry(*it, 32767));

        score = it->score;
        pos.setBoard(it->fen);
    }

    MoveList ml;
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);

    if (ml.length > 0)
        game.moves.push_back(ViriFormatMoveEntry(ml.moves[0].move, entries.rend()->score));

    out << game;
}

void processUnfilteredGame(std::vector<Entry>& entries, std::ofstream& out) {
    Position pos;
    pos.setBoard(entries.begin()->fen);
    Move m = moveFromUci(pos, entries.begin()->move);

    ViriFormatGame game(MarlinFormatPackedBoard(pos, entries.begin()->result));

    pos.makeMove(m);
    game.moves.push_back(ViriFormatMoveEntry(m, entries.begin()->score));

    for (auto it = entries.begin() + 1; it < entries.end(); it++) {
        if (pos.fen() != it->fen) {
            std::cout << "Position mismatch when going from " << pos.fen() << " to " << it->fen << " with " << (it - 1)->move << ". Skipping." << std::endl;
            return;
        }

        Move m = moveFromUci(pos, it->move);
        pos.makeMove(m);
        game.moves.push_back(ViriFormatMoveEntry(m, it->score));
    }

    out << game;

    delete pos.net;
}

Entry parseNextGame(std::ifstream& in, std::vector<Entry>& entries) {
    std::string line;

    int mc = 10000;

    while (true) {
        Entry entry;

        std::getline(in, line);
        std::vector<std::string> l = split(line, '|');

        if (in.eof())
            return entry;

        entry.fen    = l[0].substr(0, l[0].size() - 1);
        entry.score  = std::stoi(l[1].substr(1, l[1].size() - 1));
        entry.result = (l[2] == " 1.0") ? 2 : (l[2] == " 0.5") ? 1 : 0;

        int moveCount = std::stoi(split(entry.fen, ' ')[5]);

        if (moveCount >= mc)
            return entry;

        entries.push_back(entry);
        mc = moveCount;
    }
}

Entry parseNextUnfilteredGame(std::ifstream& in, std::vector<Entry>& entries) {
    std::string line;

    int mc = 0;

    while (true) {
        Entry entry;

        std::getline(in, line);
        std::vector<std::string> l = split(line, '|');

        if (in.eof())
            return entry;

        entry.fen    = l[0].substr(0, l[0].size() - 1);
        entry.score  = std::stoi(l[1].substr(1, l[1].size() - 1));
        entry.result = (l[2] == " 1.0") ? 2 : (l[2] == " 0.5") ? 1 : 0;
        entry.move   = l[3].substr(1);

        int moveCount = std::stoi(split(entry.fen, ' ')[5]);

        if (mc >= moveCount)
            return entry;

        entries.push_back(entry);
        mc = moveCount;
    }
}

bool findPos(Position& pos, Position& target, int depthDiff, std::vector<Move>& moves, bool first) {
    if (depthDiff <= 0) {
        return    pos.enPassantSquare == target.enPassantSquare
               && pos.castlingRights == target.castlingRights
               && std::equal(&pos.pieceLocations[0], &pos.pieceLocations[0] + pos.pieceLocations.size(), &target.pieceLocations[0]);
    }

    MoveList ml;
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);

    for (int i = 0; i < ml.length; i++) {
        Move m = ml.moves[i].move;

        if (extract<FLAG>(m) != PROMOTION && !pos.isCapture(m) && !first)
            continue;

        pos.makeMove(m);
        pos.movecount++;
        moves.push_back(m);

        bool found = findPos(pos, target, depthDiff - 1, moves);

        pos.movecount--;
        pos.unmakeMove(m);

        if (found)
            return true;

        moves.pop_back();
    }

    return false;
}

Move moveFromUci(Position& pos, std::string move) {
    int from       = lsb(stringToSquare(move.substr(0, 2)));
    int to         = lsb(stringToSquare(move.substr(2, 4)));
    int flag       = (move.size() == 5) ? PROMOTION : NORMAL;
    int promoPiece = flag ? charIntToPiece(toupper(move.at(4)) - '0') - 1 : PROMO_KNIGHT;

    return pos.fromToToMove(from, to, promoPiece, flag);
}