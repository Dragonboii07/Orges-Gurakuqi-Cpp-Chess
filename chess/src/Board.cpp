#include "Board.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static inline bool onBoard(int file, int rank) {
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

// (file, rank) steps; first 4 are orthogonal (rook), last 4 diagonal (bishop)
static const int kingDeltas[8][2] = {
    {1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}
};
static const int knightDeltas[8][2] = {
    {1,2}, {2,1}, {2,-1}, {1,-2}, {-1,-2}, {-2,-1}, {-2,1}, {-1,2}
};

// castlingRights &= castleMask[from] & castleMask[to]: moving or capturing on a
// king/rook home square removes the matching rights
static unsigned char castleMaskFor(int sq) {
    switch (sq) {
        case 0:  return 0b1101; // a1 rook: white Q
        case 4:  return 0b1100; // e1 king: white KQ
        case 7:  return 0b1110; // h1 rook: white K
        case 56: return 0b0111; // a8 rook: black q
        case 60: return 0b0011; // e8 king: black kq
        case 63: return 0b1011; // h8 rook: black k
        default: return 0b1111;
    }
}

// Zobrist keys, generated once from a fixed seed
struct ZobristKeys {
    uint64_t piece[13][64];
    uint64_t blackToMove;
    uint64_t castling[16];
    uint64_t epFile[8];
    ZobristKeys() {
        uint64_t s = 0x9E3779B97F4A7C15ULL;
        auto next = [&s]() {
            // splitmix64
            uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        };
        for (auto& row : piece) for (auto& k : row) k = next();
        blackToMove = next();
        for (auto& k : castling) k = next();
        for (auto& k : epFile) k = next();
    }
};
static const ZobristKeys zobrist;

Board::Board() {
    reset();
}

void Board::reset() {
    setFen(START_FEN);
}

bool Board::setFen(const std::string& fenStr) {
    std::istringstream iss(fenStr);
    std::string placement, side, castling, ep;
    if (!(iss >> placement >> side >> castling >> ep)) return false;
    int half = 0, full = 1;
    iss >> half >> full;

    std::array<Piece, 64> sq;
    sq.fill(EMPTY);
    int rank = 7, file = 0;
    for (char c : placement) {
        if (c == '/') {
            if (file != 8) return false;
            --rank;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            Piece p = pieceFromChar(c);
            if (p == EMPTY || !onBoard(file, rank)) return false;
            sq[rank*8 + file++] = p;
        }
        if (file > 8 || rank < 0) return false;
    }
    if (rank != 0 || file != 8) return false;
    if (std::count(sq.begin(), sq.end(), WK) != 1 || std::count(sq.begin(), sq.end(), BK) != 1) return false;
    if (side != "w" && side != "b") return false;

    unsigned char cr = 0;
    if (castling != "-") {
        for (char c : castling) {
            switch (c) {
                case 'K': cr |= 1; break;
                case 'Q': cr |= 2; break;
                case 'k': cr |= 4; break;
                case 'q': cr |= 8; break;
                default: return false;
            }
        }
    }
    int epSq = -1;
    if (ep != "-") {
        if (ep.size() != 2 || ep[0] < 'a' || ep[0] > 'h' || ep[1] < '1' || ep[1] > '8') return false;
        epSq = (ep[0] - 'a') + (ep[1] - '1') * 8;
    }

    squares = sq;
    whiteToMove = (side == "w");
    castlingRights = cr;
    enPassant = epSq;
    halfmoveClock = half;
    fullmoveNumber = full;
    undoStack.clear();
    history.clear();
    history.push_back(hash());
    return true;
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

// piece-square tables, written as you'd see the board from white's side:
// the first row is rank 8, the last row is rank 1. Index with (sq ^ 56) for
// white and sq for black.
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

static void addPawnMove(std::vector<Move>& moves, int from, int to, bool promotes) {
    if (promotes) {
        moves.emplace_back(from, to, 'q');
        moves.emplace_back(from, to, 'r');
        moves.emplace_back(from, to, 'b');
        moves.emplace_back(from, to, 'n');
    } else {
        moves.emplace_back(from, to);
    }
}

std::vector<Move> Board::generateMoves() const {
    std::vector<Move> moves;
    moves.reserve(64);
    bool white = whiteToMove;
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = squares[sq];
        if (p == EMPTY || isWhitePiece(p) != white) continue;
        int file = sq % 8, rank = sq / 8;
        auto isEnemy = [&](Piece q) { return q != EMPTY && isWhitePiece(q) != white; };

        switch (p) {
            case WP: case BP: {
                int dir = white ? 1 : -1;
                int promoRank = white ? 7 : 0;
                int startRank = white ? 1 : 6;
                int fr = rank + dir;
                if (!onBoard(file, fr)) break;
                int forward = fr*8 + file;
                if (squares[forward] == EMPTY) {
                    addPawnMove(moves, sq, forward, fr == promoRank);
                    int dbl = forward + dir*8;
                    if (rank == startRank && squares[dbl] == EMPTY) moves.emplace_back(sq, dbl);
                }
                for (int df : {-1, 1}) {
                    if (!onBoard(file + df, fr)) continue;
                    int t = fr*8 + file + df;
                    if (isEnemy(squares[t])) addPawnMove(moves, sq, t, fr == promoRank);
                    else if (t == enPassant) moves.emplace_back(sq, t);
                }
                break;
            }
            case WN: case BN: {
                for (auto& d : knightDeltas) {
                    int f = file + d[0], r = rank + d[1];
                    if (!onBoard(f, r)) continue;
                    Piece q = squares[r*8 + f];
                    if (q == EMPTY || isEnemy(q)) moves.emplace_back(sq, r*8 + f);
                }
                break;
            }
            case WB: case BB:
            case WR: case BR:
            case WQ: case BQ: {
                int first = (p == WB || p == BB) ? 4 : 0;
                int last  = (p == WR || p == BR) ? 4 : 8;
                for (int i = first; i < last; ++i) {
                    int f = file, r = rank;
                    while (true) {
                        f += kingDeltas[i][0];
                        r += kingDeltas[i][1];
                        if (!onBoard(f, r)) break;
                        Piece q = squares[r*8 + f];
                        if (q == EMPTY) {
                            moves.emplace_back(sq, r*8 + f);
                        } else {
                            if (isEnemy(q)) moves.emplace_back(sq, r*8 + f);
                            break;
                        }
                    }
                }
                break;
            }
            case WK: case BK: {
                for (auto& d : kingDeltas) {
                    int f = file + d[0], r = rank + d[1];
                    if (!onBoard(f, r)) continue;
                    Piece q = squares[r*8 + f];
                    if (q == EMPTY || isEnemy(q)) moves.emplace_back(sq, r*8 + f);
                }
                // castling: king may not start in or pass through check; landing
                // in check is rejected by the legality filter
                int home = white ? 4 : 60;
                if (sq != home) break;
                unsigned char kRight = white ? 1 : 4, qRight = white ? 2 : 8;
                Piece rook = white ? WR : BR;
                bool enemy = !white;
                if ((castlingRights & kRight) && squares[home+1] == EMPTY && squares[home+2] == EMPTY
                    && squares[home+3] == rook
                    && !isSquareAttacked(home, enemy) && !isSquareAttacked(home+1, enemy)) {
                    moves.emplace_back(home, home+2);
                }
                if ((castlingRights & qRight) && squares[home-1] == EMPTY && squares[home-2] == EMPTY
                    && squares[home-3] == EMPTY && squares[home-4] == rook
                    && !isSquareAttacked(home, enemy) && !isSquareAttacked(home-1, enemy)) {
                    moves.emplace_back(home, home-2);
                }
                break;
            }
            default: break;
        }
    }
    return moves;
}

std::vector<Move> Board::generateLegalMoves() {
    std::vector<Move> legal;
    bool side = whiteToMove;
    for (const Move& m : generateMoves()) {
        makeMove(m);
        if (!inCheck(side)) legal.push_back(m);
        unmakeMove(m);
    }
    return legal;
}

bool Board::isSquareAttacked(int sq, bool byWhite) const {
    int file = sq % 8, rank = sq / 8;

    // a white pawn attacks diagonally upward, so it sits one rank below
    Piece pawn = byWhite ? WP : BP;
    int pr = byWhite ? rank - 1 : rank + 1;
    for (int df : {-1, 1}) {
        if (onBoard(file + df, pr) && squares[pr*8 + file + df] == pawn) return true;
    }

    Piece knight = byWhite ? WN : BN;
    for (auto& d : knightDeltas) {
        int f = file + d[0], r = rank + d[1];
        if (onBoard(f, r) && squares[r*8 + f] == knight) return true;
    }

    Piece king = byWhite ? WK : BK;
    for (auto& d : kingDeltas) {
        int f = file + d[0], r = rank + d[1];
        if (onBoard(f, r) && squares[r*8 + f] == king) return true;
    }

    Piece rook = byWhite ? WR : BR;
    Piece bishop = byWhite ? WB : BB;
    Piece queen = byWhite ? WQ : BQ;
    for (int i = 0; i < 8; ++i) {
        Piece slider = i < 4 ? rook : bishop;
        int f = file, r = rank;
        while (true) {
            f += kingDeltas[i][0];
            r += kingDeltas[i][1];
            if (!onBoard(f, r)) break;
            Piece q = squares[r*8 + f];
            if (q == EMPTY) continue;
            if (q == slider || q == queen) return true;
            break;
        }
    }
    return false;
}

bool Board::inCheck(bool white) const {
    Piece king = white ? WK : BK;
    for (int sq = 0; sq < 64; ++sq) {
        if (squares[sq] == king) return isSquareAttacked(sq, !white);
    }
    return false;
}

bool Board::isCapture(const Move& m) const {
    if (squares[m.to] != EMPTY) return true;
    Piece p = squares[m.from];
    return (p == WP || p == BP) && m.to == enPassant;
}

void Board::makeMove(const Move& m) {
    Piece p = squares[m.from];
    bool isPawn = (p == WP || p == BP);
    Undo u{p, squares[m.to], m.to, castlingRights, enPassant, halfmoveClock};

    // en passant: the captured pawn is behind the target square
    if (isPawn && m.to == enPassant) {
        u.capturedSq = m.to + (whiteToMove ? -8 : 8);
        u.captured = squares[u.capturedSq];
        squares[u.capturedSq] = EMPTY;
    }
    undoStack.push_back(u);

    squares[m.to] = p;
    squares[m.from] = EMPTY;
    if (m.promotion) {
        char c = whiteToMove ? toupper(m.promotion) : tolower(m.promotion);
        squares[m.to] = pieceFromChar(c);
    }
    // castling: king moves two files, bring the rook across
    if ((p == WK || p == BK) && std::abs(m.to - m.from) == 2) {
        if (m.to > m.from) {
            squares[m.from + 1] = squares[m.from + 3];
            squares[m.from + 3] = EMPTY;
        } else {
            squares[m.from - 1] = squares[m.from - 4];
            squares[m.from - 4] = EMPTY;
        }
    }
    castlingRights &= castleMaskFor(m.from) & castleMaskFor(m.to);

    enPassant = -1;
    if (isPawn && std::abs(m.to - m.from) == 16) enPassant = (m.from + m.to) / 2;

    halfmoveClock = (isPawn || u.captured != EMPTY) ? 0 : halfmoveClock + 1;
    if (!whiteToMove) fullmoveNumber++;
    whiteToMove = !whiteToMove;
    history.push_back(hash());
}

void Board::unmakeMove(const Move& m) {
    Undo u = undoStack.back();
    undoStack.pop_back();
    history.pop_back();

    whiteToMove = !whiteToMove;
    if (!whiteToMove) fullmoveNumber--;
    castlingRights = u.castlingRights;
    enPassant = u.enPassant;
    halfmoveClock = u.halfmoveClock;

    // restoring u.moved (not what's on m.to) also undoes promotions
    squares[m.from] = u.moved;
    squares[m.to] = EMPTY;
    squares[u.capturedSq] = u.captured;

    if ((u.moved == WK || u.moved == BK) && std::abs(m.to - m.from) == 2) {
        if (m.to > m.from) {
            squares[m.from + 3] = squares[m.from + 1];
            squares[m.from + 1] = EMPTY;
        } else {
            squares[m.from - 4] = squares[m.from - 1];
            squares[m.from - 1] = EMPTY;
        }
    }
}

int Board::evaluate() const {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        Piece p = squares[i];
        int w = i ^ 56; // flip rank so white reads the tables from its side
        switch (p) {
            case WP: score += 100 + pawnTable[w]; break;
            case WN: score += 320 + knightTable[w]; break;
            case WB: score += 330 + bishopTable[w]; break;
            case WR: score += 500 + rookTable[w]; break;
            case WQ: score += 900 + queenTable[w]; break;
            case WK: score += kingTable[w]; break;
            case BP: score -= 100 + pawnTable[i]; break;
            case BN: score -= 320 + knightTable[i]; break;
            case BB: score -= 330 + bishopTable[i]; break;
            case BR: score -= 500 + rookTable[i]; break;
            case BQ: score -= 900 + queenTable[i]; break;
            case BK: score -= kingTable[i]; break;
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
    s += ' ' + std::to_string(halfmoveClock) + ' ' + std::to_string(fullmoveNumber);
    return s;
}

uint64_t Board::hash() const {
    uint64_t h = 0;
    for (int sq = 0; sq < 64; ++sq) {
        if (squares[sq] != EMPTY) h ^= zobrist.piece[squares[sq]][sq];
    }
    if (!whiteToMove) h ^= zobrist.blackToMove;
    h ^= zobrist.castling[castlingRights];
    if (enPassant >= 0) h ^= zobrist.epFile[enPassant % 8];
    return h;
}

// how many times the current position has occurred; only positions since the
// last capture or pawn move can repeat, and only with the same side to move
int Board::repetitionCount() const {
    int count = 0;
    int n = (int)history.size();
    uint64_t cur = history.back();
    for (int i = n - 1; i >= 0 && i >= n - 1 - halfmoveClock; i -= 2) {
        if (history[i] == cur) count++;
    }
    return count;
}

bool Board::isThreefold() const {
    return repetitionCount() >= 3;
}

bool Board::isFiftyMove() const {
    return halfmoveClock >= 100;
}

uint64_t Board::perft(int depth) {
    if (depth == 0) return 1;
    auto moves = generateLegalMoves();
    if (depth == 1) return moves.size();
    uint64_t total = 0;
    for (const Move& m : moves) {
        makeMove(m);
        total += perft(depth - 1);
        unmakeMove(m);
    }
    return total;
}

std::string Board::toSan(const Move& m) {
    Piece p = squares[m.from];
    auto squareName = [](int sq) {
        return std::string{char('a' + sq % 8), char('1' + sq / 8)};
    };

    std::string san;
    if ((p == WK || p == BK) && std::abs(m.to - m.from) == 2) {
        san = m.to > m.from ? "O-O" : "O-O-O";
    } else if (p == WP || p == BP) {
        if (isCapture(m)) {
            san += char('a' + m.from % 8);
            san += 'x';
        }
        san += squareName(m.to);
        if (m.promotion) {
            san += '=';
            san += char(toupper(m.promotion));
        }
    } else {
        san += char(toupper(pieceToChar(p)));
        // if another piece of the same kind can reach the same square, add
        // the file, else the rank, else both, of the piece that moves
        bool clash = false, sameFile = false, sameRank = false;
        for (const Move& o : generateLegalMoves()) {
            if (o.to != m.to || o.from == m.from || squares[o.from] != p) continue;
            clash = true;
            if (o.from % 8 == m.from % 8) sameFile = true;
            if (o.from / 8 == m.from / 8) sameRank = true;
        }
        if (clash) {
            if (!sameFile) san += char('a' + m.from % 8);
            else if (!sameRank) san += char('1' + m.from / 8);
            else san += squareName(m.from);
        }
        if (isCapture(m)) san += 'x';
        san += squareName(m.to);
    }

    makeMove(m);
    if (inCheck(whiteToMove)) san += generateLegalMoves().empty() ? '#' : '+';
    unmakeMove(m);
    return san;
}

// drop the decorations people type inconsistently: x, +, #, =, !, ?
static std::string normalizeSan(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == 'x' || c == 'X' || c == '+' || c == '#' || c == '=' || c == '!' || c == '?') continue;
        out += (c == '0') ? 'O' : c;
    }
    return out;
}

static std::string lowerCase(std::string s) {
    for (char& c : s) c = tolower(c);
    return s;
}

bool Board::parseMove(const std::string& text, Move& out) {
    auto legal = generateLegalMoves();
    std::string want = normalizeSan(text);
    std::string wantLower = lowerCase(want);

    // exact-case SAN wins: "bxc3" is a pawn capture, "Bxc3" a bishop move
    const Move* loose = nullptr;
    int looseCount = 0;
    for (const Move& m : legal) {
        std::string san = normalizeSan(toSan(m));
        if (san == want) { out = m; return true; }
        // "e8" for a promotion means queen
        if (m.promotion == 'q' && san == want + "Q") { out = m; return true; }
        if (lowerCase(san) == wantLower || (m.promotion == 'q' && lowerCase(san) == wantLower + "q")) {
            loose = &m;
            looseCount++;
        }
    }
    if (looseCount == 1) { out = *loose; return true; }

    // coordinates: e2e4, e7e8q
    std::string lower = lowerCase(text);
    for (const Move& m : legal) {
        std::string coord = m.toString();
        if (coord == lower || (m.promotion == 'q' && coord == lower + "q")) {
            out = m;
            return true;
        }
    }
    return false;
}
