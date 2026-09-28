#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include "Move.h"

enum Piece {
    EMPTY = 0,
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK
};

inline bool isWhitePiece(Piece p) { return p >= WP && p <= WK; }
inline bool isBlackPiece(Piece p) { return p >= BP && p <= BK; }

// everything makeMove changes that can't be recomputed from the move itself
struct Undo {
    Piece moved;
    Piece captured;
    int capturedSq;          // differs from move.to for en passant
    unsigned char castlingRights;
    int enPassant;
    int halfmoveClock;
};

struct Board {
    // 0..63 squares, 0=a1, 7=h1, 56=a8,63=h8
    std::array<Piece, 64> squares;
    bool whiteToMove;
    // castling rights bits: 1=K, 2=Q, 4=k, 8=q
    unsigned char castlingRights;
    // en passant target square (0-63) or -1
    int enPassant;
    // plies since the last capture or pawn move (50-move rule)
    int halfmoveClock;
    int fullmoveNumber;
    // hash of every position reached, for repetition detection
    std::vector<uint64_t> history;
    std::vector<Undo> undoStack;

    Board();
    void reset();
    bool setFen(const std::string& fen);
    std::vector<Move> generateMoves() const;   // pseudo-legal: may leave own king in check
    std::vector<Move> generateLegalMoves();
    void makeMove(const Move& m);
    void unmakeMove(const Move& m);
    bool isSquareAttacked(int sq, bool byWhite) const;
    bool inCheck(bool white) const;
    bool isCapture(const Move& m) const;
    int evaluate() const;                      // centipawns, positive = good for white
    void print() const;
    std::string fen() const;
    uint64_t hash() const;
    int repetitionCount() const;
    bool isThreefold() const;
    bool isFiftyMove() const;
    uint64_t perft(int depth);
    // standard algebraic notation (Nf3, exd5, O-O, e8=Q+); m must be legal
    std::string toSan(const Move& m);
    // accepts SAN (lenient about case, x, +, #, =, 0-0) or coordinates (e2e4)
    bool parseMove(const std::string& text, Move& out);
    static Piece pieceFromChar(char c);
    static char pieceToChar(Piece p);
};
