#include "Board.h"
#include <iostream>

Board::Board() {
    reset();
}

void Board::reset() {
    whiteToMove = true;
    castlingRights = 0b1111; // KQkq available
    enPassant = -1;
    history.clear();
    // initial position
    const Piece init[64] = {
        WR, WN, WB, WQ, WK, WB, WN, WR,
        WP, WP, WP, WP, WP, WP, WP, WP,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        BP, BP, BP, BP, BP, BP, BP, BP,
        BR, BN, BB, BQ, BK, BB, BN, BR
    };
    for (int i = 0; i < 64; ++i) squares[i] = init[i];
    history.push_back(fen());
}

char Board::pieceToChar(Piece p) {
    switch (p) {
        case WP: return 'P';
        case WN: return 'N';
        case WB: return 'B';
        case WR: return 'R';
        case WQ: return 'Q';
        case WK: return 'K';
        case BP: return 'p';
        case BN: return 'n';
        case BB: return 'b';
        case BR: return 'r';
        case BQ: return 'q';
        case BK: return 'k';
        default: return '.';
    }
}

Piece Board::pieceFromChar(char c) {
    switch (c) {
        case 'P': return WP;
        case 'N': return WN;
        case 'B': return WB;
        case 'R': return WR;
        case 'Q': return WQ;
        case 'K': return WK;
        case 'p': return BP;
        case 'n': return BN;
        case 'b': return BB;
        case 'r': return BR;
        case 'q': return BQ;
        case 'k': return BK;
        default: return EMPTY;
    }
}

void Board::print() const {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank+1) << " ";
        for (int file = 0; file < 8; ++file) {
            Piece p = squares[rank*8 + file];
            std::cout << pieceToChar(p) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "  a b c d e f g h\n";
}

// simple move generation with special rules
static const int knightOffsets[8] = {17, 15, 10, 6, -17, -15, -10, -6};
static const int kingOffsets[8] = {1, -1, 8, -8, 9, 7, -9, -7};

// piece-square tables (white perspective); we'll mirror for black by flipping index
static const int pawnTable[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};
static const int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};
static const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};
static const int rookTable[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};
static const int queenTable[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};
static const int kingTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};
std::vector<Move> Board::generateMoves() const {
    std::vector<Move> moves;
    bool white = whiteToMove;
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = squares[sq];
        if (p == EMPTY) continue;
        bool isWhite = (p <= WK && p >= WP);
        if (isWhite != white) continue;
        int dir = isWhite ? 1 : -1;
        switch (p) {
            case WP:
            case BP: {
                int forward = sq + dir*8;
                if (forward >=0 && forward < 64 && squares[forward]==EMPTY) {
                    // promotion
                    int rank = forward/8;
                    if (rank==0 || rank==7) {
                        moves.emplace_back(sq, forward, 'q');
                        moves.emplace_back(sq, forward, 'r');
                        moves.emplace_back(sq, forward, 'b');
                        moves.emplace_back(sq, forward, 'n');
                    } else {
                        moves.emplace_back(sq, forward);
                    }
                    // double move
                    int startRank = isWhite ? 1 : 6;
                    if (sq/8 == startRank) {
                        int dbl = sq + dir*16;
                        if (squares[dbl] == EMPTY) moves.emplace_back(sq, dbl);
                    }
                }
                // captures
                int caps[2] = {sq + dir*8 + 1, sq + dir*8 - 1};
                for (int i =0;i<2;++i) {
                    int t = caps[i];
                    if (t>=0 && t<64) {
                        Piece q = squares[t];
                        if (q!=EMPTY) {
                            bool qWhite = (q<=WK && q>=WP);
                            if (qWhite!=isWhite) {
                                int rank = t/8;
                                if (rank==0||rank==7) {
                                    moves.emplace_back(sq,t,'q');
                                    moves.emplace_back(sq,t,'r');
                                    moves.emplace_back(sq,t,'b');
                                    moves.emplace_back(sq,t,'n');
                                } else {
                                    moves.emplace_back(sq,t);
                                }
                            }
                        }
                    }
                }
                // en passant capture
                if (enPassant >= 0) {
                    int epRank = enPassant/8;
                    int epFile = enPassant%8;
                    int sqRank = sq/8;
                    if ((sqRank + dir) == epRank && abs((sq%8) - epFile) == 1) {
                        moves.emplace_back(sq, enPassant);
                    }
                }
                break;
            }
            case WN: case BN: {
                for (int k: knightOffsets) {
                    int t = sq + k;
                    if (t<0 || t>=64) continue;
                    int df = (t%8) - (sq%8);
                    int dr = (t/8) - (sq/8);
                    if (abs(df)>2||abs(dr)>2) continue;
                    Piece q = squares[t];
                    if (q==EMPTY || ((q<=WK) != isWhite)) {
                        moves.emplace_back(sq,t);
                    }
                }
                break;
            }
            case WB: case BB: {
                int directions[4] = {9,7,-9,-7};
                for (int d: directions) {
                    int t = sq;
                    while (1) {
                        t += d;
                        if (t<0||t>=64) break;
                        int df = (t%8) - ((t-d)%8);
                        if (abs(df)>1) break; // wrap
                        Piece q = squares[t];
                        if (q==EMPTY) {
                            moves.emplace_back(sq,t);
                        } else {
                            if ((q<=WK)!=isWhite) moves.emplace_back(sq,t);
                            break;
                        }
                    }
                }
                break;
            }
            case WR: case BR: {
                int directions[4] = {1,-1,8,-8};
                for (int d: directions) {
                    int t = sq;
                    while (1) {
                        t += d;
                        if (t<0||t>=64) break;
                        int df = (t%8) - ((t-d)%8);
                        if (abs(d)==1 && abs(df)>1) break;
                        Piece q = squares[t];
                        if (q==EMPTY) {
                            moves.emplace_back(sq,t);
                        } else {
                            if ((q<=WK)!=isWhite) moves.emplace_back(sq,t);
                            break;
                        }
                    }
                }
                break;
            }
            case WQ: case BQ: {
                int directions[8] = {1,-1,8,-8,9,7,-9,-7};
                for (int d: directions) {
                    int t = sq;
                    while (1) {
                        t += d;
                        if (t<0||t>=64) break;
                        int df = (t%8) - ((t-d)%8);
                        if (abs(d)==1 && abs(df)>1) break;
                        Piece q = squares[t];
                        if (q==EMPTY) {
                            moves.emplace_back(sq,t);
                        } else {
                            if ((q<=WK)!=isWhite) moves.emplace_back(sq,t);
                            break;
                        }
                    }
                }
                break;
            }
            case WK: case BK: {
                for (int k: kingOffsets) {
                    int t = sq + k;
                    if (t<0||t>=64) continue;
                    int df = (t%8) - (sq%8);
                    if (abs(df)>1) continue;
                    Piece q = squares[t];
                    if (q==EMPTY || ((q<=WK) != isWhite)) {
                        moves.emplace_back(sq,t);
                    }
                }
                // castling (simplified: ignore checks)
                if (isWhite && sq==4 && whiteToMove) {
                    if (castlingRights & 1) { // K
                        if (squares[5]==EMPTY && squares[6]==EMPTY) {
                            moves.emplace_back(4,6); // king-side
                        }
                    }
                    if (castlingRights & 2) { // Q
                        if (squares[3]==EMPTY && squares[2]==EMPTY && squares[1]==EMPTY) {
                            moves.emplace_back(4,2); // queen-side
                        }
                    }
                }
                if (!isWhite && sq==60 && !whiteToMove) {
                    if (castlingRights & 4) { // k
                        if (squares[61]==EMPTY && squares[62]==EMPTY) {
                            moves.emplace_back(60,62);
                        }
                    }
                    if (castlingRights & 8) { // q
                        if (squares[59]==EMPTY && squares[58]==EMPTY && squares[57]==EMPTY) {
                            moves.emplace_back(60,58);
                        }
                    }
                }
                break;
            }
            default: break;
        }
    }
    return moves;
}

bool Board::makeMove(const Move& m) {
    Piece p = squares[m.from];
    Piece dest = squares[m.to];
    // save previous state for unmake
    unsigned char prevCastling = castlingRights;
    int prevEP = enPassant;

    // handle en passant capture
    if ((p == WP || p == BP) && m.to == enPassant) {
        int dir = whiteToMove ? 1 : -1;
        int capSq = m.to - dir*8;
        dest = squares[capSq];
        squares[capSq] = EMPTY;
    }

    // move piece
    squares[m.to] = p;
    squares[m.from] = EMPTY;
    // promotion
    if (m.promotion) {
        squares[m.to] = whiteToMove ? Board::pieceFromChar(toupper(m.promotion)) : Board::pieceFromChar(tolower(m.promotion));
    }
    // castling rook move
    if (p == WK && m.from == 4) {
        if (m.to == 6) { // white king-side
            squares[5] = WR;
            squares[7] = EMPTY;
        } else if (m.to == 2) { // queen-side
            squares[3] = WR;
            squares[0] = EMPTY;
        }
        castlingRights &= ~0b0011; // clear white rights
    }
    if (p == BK && m.from == 60) {
        if (m.to == 62) {
            squares[61] = BR;
            squares[63] = EMPTY;
        } else if (m.to == 58) {
            squares[59] = BR;
            squares[56] = EMPTY;
        }
        castlingRights &= ~0b1100; // clear black rights
    }
    // rook moves/captures affecting castling rights
    if (p == WR) {
        if (m.from == 0) castlingRights &= ~0b0010; // white Q
        if (m.from == 7) castlingRights &= ~0b0001; // white K
    }
    if (p == BR) {
        if (m.from == 56) castlingRights &= ~0b1000; // black q
        if (m.from == 63) castlingRights &= ~0b0100; // black k
    }
    if (dest == WR) {
        if (m.to == 0) castlingRights &= ~0b0010;
        if (m.to == 7) castlingRights &= ~0b0001;
    }
    if (dest == BR) {
        if (m.to == 56) castlingRights &= ~0b1000;
        if (m.to == 63) castlingRights &= ~0b0100;
    }

    // set en passant target
    enPassant = -1;
    if (p == WP || p == BP) {
        int rankFrom = m.from / 8;
        int rankTo = m.to / 8;
        if (abs(rankTo - rankFrom) == 2) {
            int ep = (m.from + m.to) / 2;
            enPassant = ep;
        }
    }

    whiteToMove = !whiteToMove;
    history.push_back(fen());
    return true;
}

void Board::unmakeMove(const Move& m, Piece captured, bool prevWhite, unsigned char prevCastling, int prevEP) {
    // restore whiteToMove and state
    whiteToMove = prevWhite;
    castlingRights = prevCastling;
    enPassant = prevEP;

    Piece p = squares[m.to];
    // undo castling rook
    if (p == WK && m.from == 4) {
        if (m.to == 6) {
            squares[7] = WR;
            squares[5] = EMPTY;
        } else if (m.to == 2) {
            squares[0] = WR;
            squares[3] = EMPTY;
        }
    }
    if (p == BK && m.from == 60) {
        if (m.to == 62) {
            squares[63] = BR;
            squares[61] = EMPTY;
        } else if (m.to == 58) {
            squares[56] = BR;
            squares[59] = EMPTY;
        }
    }
    // restore pieces
    squares[m.from] = p;
    squares[m.to] = captured;
    // en passant capture restore
    if ((p == WP || p == BP) && m.to == prevEP && captured == EMPTY) {
        int dir = prevWhite ? -1 : 1; // previous side
        int capSq = m.to - dir*8;
        squares[capSq] = (prevWhite ? BP : WP);
    }
    // remove last history entry
    if (!history.empty()) history.pop_back();
}

int Board::evaluate() const {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        Piece p = squares[i];
        switch (p) {
            case WP: score += 100 + pawnTable[i]; break;
            case WN: score += 320 + knightTable[i]; break;
            case WB: score += 330 + bishopTable[i]; break;
            case WR: score += 500 + rookTable[i]; break;
            case WQ: score += 900 + queenTable[i]; break;
            case WK: score += 20000 + kingTable[i]; break;
            case BP: score -= 100 + pawnTable[63 - i]; break;
            case BN: score -= 320 + knightTable[63 - i]; break;
            case BB: score -= 330 + bishopTable[63 - i]; break;
            case BR: score -= 500 + rookTable[63 - i]; break;
            case BQ: score -= 900 + queenTable[63 - i]; break;
            case BK: score -= 20000 + kingTable[63 - i]; break;
            default: break;
        }
    }
    return score;
}

std::string Board::fen() const {
    std::string s;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            Piece p = squares[rank*8 + file];
            if (p == EMPTY) {
                empty++;
            } else {
                if (empty) {
                    s += char('0' + empty);
                    empty = 0;
                }
                s += pieceToChar(p);
            }
        }
        if (empty) s += char('0' + empty);
        if (rank) s += '/';
    }
    s += ' ';
    s += (whiteToMove ? 'w' : 'b');
    s += ' ';
    std::string cr;
    if (castlingRights & 1) cr += 'K';
    if (castlingRights & 2) cr += 'Q';
    if (castlingRights & 4) cr += 'k';
    if (castlingRights & 8) cr += 'q';
    if (cr.empty()) cr = "-";
    s += cr;
    s += ' ';
    if (enPassant >= 0) {
        char file = 'a' + (enPassant % 8);
        char rank = '1' + (enPassant / 8);
        s.push_back(file);
        s.push_back(rank);
    } else {
        s += '-';
    }
    return s;
}

bool Board::isThreefold() const {
    if (history.empty()) return false;
    std::string cur = history.back();
    int count = 0;
    for (auto &h : history) {
        if (h == cur) count++;
    }
    return count >= 3;
}
