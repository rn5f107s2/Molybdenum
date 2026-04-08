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
};

static int skipped = 0;

Entry parseNextGame(std::ifstream& in, std::vector<Entry>& entries);
void processGame(std::vector<Entry>& entries, std::ofstream& out);
bool findPos(Position& pos, Position& target, int depthDiff, std::vector<Move>& moves, bool first = false);

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