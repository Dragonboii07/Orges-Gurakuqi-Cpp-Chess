#pragma once

#include <array>
#include <vector>
#include <string>
#include "Move.h"

enum Piece {
    EMPTY = 0,
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK
};

struct Board {
    // 0..63 squares, 0=a1, 7=h1, 56=a8,63=h8
    std::array<Piece, 64> squares;
    bool whiteToMove;
    // castling rights bits: white KQ, black kq
    unsigned char castlingRights;
    // en passant target square (0-63) or -1
    int enPassant;
    // simple history of position strings for repetition
    std::vector<std::string> history;

    Board();
    void reset();
    std::vector<Move> generateMoves() const;
    bool makeMove(const Move& m);
    void unmakeMove(const Move& m, Piece captured, bool prevWhite, unsigned char prevCastling, int prevEP);
    int evaluate() const;
    void print() const;
    std::string fen() const;
    bool isThreefold() const;
    static Piece pieceFromChar(char c);
    static char pieceToChar(Piece p);
};
