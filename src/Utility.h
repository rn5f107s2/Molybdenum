#ifndef MOLYBDENUM_UTILITY_H
#define MOLYBDENUM_UTILITY_H

#include <array>

constexpr int MAX_STACK_SIZE = 6000; //Longer then the longest possible chess game

template<typename T>
class Stack {
    public:
        T top();
        T pop();
        T at(int idx);
        void push(T t);
        void clear();
        int getSize();
    private:
        std::array<T, MAX_STACK_SIZE> stack;
        int size = 0;
};

template<typename T>
int Stack<T>::getSize() {
    return size;
}

template<typename T>
void Stack<T>::clear() {
    size = 0;
}

template<typename T>
void Stack<T>::push(T t) {
    stack[size] = t;
    size++;
}

template<typename T>
T Stack<T>::at(int idx) {
    return stack[idx];
}

template<typename T>
T Stack<T>::pop() {
    return stack[--size];
}

template<typename T>
T Stack<T>::top() {
    return stack[size - 1];
}

inline Piece charIntToPiece(int charInt) {
    char piece = char(charInt + '0');
    switch (piece) {
        case 'P':
            return WHITE_PAWN;
        case 'N':
            return WHITE_KNIGHT;
        case 'B':
            return WHITE_BISHOP;
        case 'R':
            return WHITE_ROOK;
        case 'Q':
            return WHITE_QUEEN;
        case 'K':
            return WHITE_KING;
        case 'p':
            return  BLACK_PAWN;
        case 'n':
            return BLACK_KNIGHT;
        case 'b':
            return BLACK_BISHOP;
        case 'r':
            return BLACK_ROOK;
        case 'q':
            return BLACK_QUEEN;
        case 'k':
            return BLACK_KING;
        default:
            return NO_PIECE;
    }
}

inline int castlingCharToIndex(int charInt) {
    switch (charInt) {
        case WHITE_KING:
            return 0;
        case WHITE_QUEEN:
            return 1;
        case BLACK_KING:
            return 2;
        default:
            return 3;

    }
}

inline char pieceToChar(int piece) {
    switch (piece) {
        case WHITE_PAWN:
            return 'P';
        case WHITE_KNIGHT:
            return 'N';
        case WHITE_BISHOP:
            return 'B';
        case WHITE_ROOK:
            return 'R';
        case WHITE_QUEEN:
            return 'Q';
        case WHITE_KING:
            return 'K';
        case BLACK_PAWN:
            return  'p';
        case BLACK_KNIGHT:
            return 'n';
        case BLACK_BISHOP:
            return 'b';
        case BLACK_ROOK:
            return 'r';
        case BLACK_QUEEN:
            return 'q';
        case BLACK_KING:
            return 'k';
        default:
            return ' ';
    }
}

inline u64 stringToSquare(std::string epSquare) {
    int file = 7 - (epSquare.at(0) - 'a');
    int  row = (epSquare.at(1) - '0') - 1;

    return 1ULL << (row * 8 + file);
}

inline int stringRoRule50(const std::string& rule50) {
    return std::stoi(rule50);
}

#endif //MOLYBDENUM_UTILITY_H
