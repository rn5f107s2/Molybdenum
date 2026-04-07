#pragma once

#include "../Move.h"
#include "../Position.h"

#include <cstdint> 
#include <vector>
#include <fstream>

using ViriFormatMove = uint16_t;

ViriFormatMove toVFMove(Move move);
uint64_t reverseBitsInBytes64(uint64_t source);

struct MarlinFormatPackedBoard {
    uint64_t occupied;

    uint8_t pieces[16];
    uint8_t epSquare;
    uint8_t fiftyMoveRule;
    uint16_t moveCount;
    int16_t score;
    uint8_t result;

    uint8_t padding;

    MarlinFormatPackedBoard(Position& pos, int r, int16_t s = 0) {
        // Moly uses h1 = 0, a1 = 0 is more widespread, so convert in order to keep compatibility
        // with other tools
        uint64_t occ = reverseBitsInBytes64(pos.getOccupied());

        occupied = occ;

        memset(pieces, 0, sizeof(pieces));

        for (int i = 0; i < 32 && occ; i++) {
            int sq     = popLSB(occ) ^ 7;
            int molyPc = pos.pieceOn(sq);
            int viriPc = typeOf(molyPc);

            if (typeOf(molyPc) == ROOK && (castlingMask[sq] & pos.castlingRights))
                viriPc = 6;

            if (colorOf(molyPc) == BLACK)
                viriPc |= 0b1000;

            if (~i & 1)
                viriPc <<= 4;

            pieces[i / 2] |= viriPc;
        }

        epSquare = !epSquare ? 64 : (lsb(pos.enPassantSquare) ^ 7);

        if (pos.sideToMove == BLACK)
            epSquare |= 1ULL << 7;

        fiftyMoveRule = pos.plys50moveRule;
        moveCount     = pos.movecount;
        score         = s;
        result        = r;

        padding = 0x5f;
    }

    friend std::ostream& operator<<(std::ostream& out, MarlinFormatPackedBoard& b) {
        out.write(reinterpret_cast<char*>(&b.occupied     ), sizeof(b.occupied));

        out.write(reinterpret_cast<char*>(b.pieces        ), sizeof(b.pieces));
        out.write(reinterpret_cast<char*>(&b.epSquare     ), sizeof(b.epSquare));
        out.write(reinterpret_cast<char*>(&b.fiftyMoveRule), sizeof(b.fiftyMoveRule));
        out.write(reinterpret_cast<char*>(&b.moveCount    ), sizeof(b.moveCount));
        out.write(reinterpret_cast<char*>(&b.score        ), sizeof(b.score));
        out.write(reinterpret_cast<char*>(&b.result       ), sizeof(b.result));

        out.write(reinterpret_cast<char*>(&b.padding      ), sizeof(b.padding));

        return out;
    }
};

struct ViriFormatMoveEntry {
    ViriFormatMove move;
    int16_t score;

    ViriFormatMoveEntry(Move m, int16_t s) {
        move  = toVFMove(m);
        score = s;
    }

    friend std::ostream& operator<<(std::ostream& out, ViriFormatMoveEntry& e) {
        out.write(reinterpret_cast<char*>(&e.move) , sizeof(e.move));
        out.write(reinterpret_cast<char*>(&e.score), sizeof(e.score)); 

        return out;
    }
};

struct ViriFormatGame {
    MarlinFormatPackedBoard board;

    std::vector<ViriFormatMoveEntry> moves;

    const int32_t zeroTerminatation = 0;

    ViriFormatGame(MarlinFormatPackedBoard b) : board(b) {}

    friend std::ostream& operator<<(std::ostream& out, ViriFormatGame& game) {
        out << game.board;

        for (ViriFormatMoveEntry& move : game.moves)
            out << move;

        out.write(reinterpret_cast<const char*>(&game.zeroTerminatation), sizeof(int32_t));

        return out;
    }
};
 
inline ViriFormatMove toVFMove(Move move) {
    ViriFormatMove m = 0;

    int from  = extract<FROM         >(move);
    int to    = extract<TO           >(move);
    int promo = extract<PROMOTIONTYPE>(move);
    int flag  = extract<FLAG>         (move);

    int vfType = (flag == ENPASSANT) ? 1 : (flag == CASTLING) ? 2 : (flag == PROMOTION) ? 3 : 0;

    if (flag == CASTLING)
        to = ((1ULL << to) & lsb(FILEG)) ? createSquare(0, rankOf(to)) : createSquare(7, rankOf(to));

    // Moly uses h1 = 0, a1 = 0 is more widespread, so convert in order to keep compatibility
    // with other tools

    from ^= 7;
    to   ^= 7;

    m |= from | (to << 6);
    m |= promo << 12;
    m |= vfType << 14;

    return m;
}

// https://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
inline unsigned char reverseBitsInByte(unsigned char byte) {
    return ((byte * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
}

inline uint64_t reverseBitsInBytes64(uint64_t source) {
    uint64_t target = 0;

    for (int i = 0; i < 8; i++)
        reinterpret_cast<unsigned char*>(&target)[i] = reverseBitsInByte(reinterpret_cast<unsigned char*>(&source)[i]);

    return target;
}
