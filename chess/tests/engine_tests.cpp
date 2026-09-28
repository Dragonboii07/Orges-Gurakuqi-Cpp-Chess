// Move generator and search tests. Perft counts every legal move sequence to a
// fixed depth; the expected numbers are the well-known published values from
// https://www.chessprogramming.org/Perft_Results, so any rule bug shows up as
// a mismatch.
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include "../src/Board.h"
#include "../src/Engine.h"

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "  pass  " : "  FAIL  ") << what << "\n";
    if (!ok) failures++;
}

struct PerftCase {
    const char* name;
    const char* fen;
    int depth;
    uint64_t expected;
};

static const PerftCase perftCases[] = {
    {"start position",  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609},
    {"kiwipete",        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603},
    {"position 3",      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624},
    {"position 4",      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333},
    {"position 5",      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487},
    {"position 6",      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594},
};

static void perftTests() {
    std::cout << "perft\n";
    for (const auto& c : perftCases) {
        Board b;
        b.setFen(c.fen);
        std::string before = b.fen();
        auto t0 = std::chrono::steady_clock::now();
        uint64_t n = b.perft(c.depth);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
        check(n == c.expected, std::string(c.name) + " depth " + std::to_string(c.depth) + ": " +
              std::to_string(n) + " (expected " + std::to_string(c.expected) + ", " + std::to_string(ms) + "ms)");
        // every make/unmake pair must leave the board exactly as it was
        check(b.fen() == before && b.history.size() == 1, std::string(c.name) + " board restored after perft");
    }
}

static std::string engineMove(const char* fen, int depth) {
    Board b;
    b.setFen(fen);
    Engine e;
    e.verbose = false;
    return e.findBestMove(b, depth).toString();
}

static void searchTests() {
    std::cout << "search\n";
    check(engineMove("6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1", 3) == "a1a8", "finds back-rank mate in 1");
    check(engineMove("r5k1/5ppp/8/8/8/8/5PPP/6K1 b - - 0 1", 3) == "a8a1", "finds mate in 1 for black");
    check(engineMove("4k3/8/8/8/8/8/3q4/R3K3 w - - 0 1", 2) == "e1d2", "takes a hanging queen with the king");
    check(engineMove("7k/P7/8/8/8/8/8/K7 w - - 0 1", 2) == "a7a8q", "promotes to a queen");

    Board b;
    b.setFen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    check(b.generateLegalMoves().empty() && !b.inCheck(false), "detects stalemate");
    b.setFen("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    check(b.generateLegalMoves().empty() && b.inCheck(false), "detects checkmate");

    b.reset();
    const char* shuffle[] = {"g1f3", "g8f6", "f3g1", "f6g8", "g1f3", "g8f6", "f3g1", "f6g8"};
    for (const char* s : shuffle) {
        for (const Move& m : b.generateLegalMoves()) {
            if (m.toString() == s) { b.makeMove(m); break; }
        }
    }
    check(b.isThreefold(), "detects threefold repetition");
}

static std::string san(const char* fen, const char* coord) {
    Board b;
    b.setFen(fen);
    for (const Move& m : b.generateLegalMoves()) {
        if (m.toString() == coord) return b.toSan(m);
    }
    return "(illegal)";
}

// what the text parses to, as coordinates, or "none"
static std::string parsed(const char* fen, const char* text) {
    Board b;
    b.setFen(fen);
    Move m(0, 0);
    return b.parseMove(text, m) ? m.toString() : "none";
}

static void notationTests() {
    std::cout << "notation\n";
    const char* start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const char* kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    check(san(start, "e2e4") == "e4", "pawn push is e4");
    check(san(start, "g1f3") == "Nf3", "knight move is Nf3");
    check(san(kiwipete, "e1g1") == "O-O" && san(kiwipete, "e1c1") == "O-O-O", "castling is O-O / O-O-O");
    check(san(kiwipete, "d5e6") == "dxe6", "pawn capture is dxe6");
    check(san(kiwipete, "f3f6") == "Qxf6", "queen capture is Qxf6");
    check(san("4k3/8/8/8/8/8/4K3/R6R w - - 0 1", "a1d1") == "Rad1", "file disambiguation Rad1");
    check(san("4k3/8/8/R7/8/8/8/R3K3 w - - 0 1", "a1a3") == "R1a3", "rank disambiguation R1a3");
    check(san("7k/P7/8/8/8/8/8/K7 w - - 0 1", "a7a8q") == "a8=Q+", "promotion with check is a8=Q+");
    check(san("6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1", "a1a8") == "Ra8#", "mate is Ra8#");

    check(parsed(start, "Nf3") == "g1f3", "reads Nf3");
    check(parsed(start, "nf3") == "g1f3", "reads nf3 (lowercase)");
    check(parsed(start, "e2e4") == "e2e4", "still reads e2e4");
    check(parsed(kiwipete, "0-0") == "e1g1" && parsed(kiwipete, "o-o-o") == "e1c1", "reads 0-0 and o-o-o");
    check(parsed(kiwipete, "Qf6") == "f3f6" && parsed(kiwipete, "Qxf6+") == "f3f6", "x and + are optional");
    check(parsed("7k/P7/8/8/8/8/8/K7 w - - 0 1", "a8") == "a7a8q", "a8 promotes to queen");
    check(parsed("7k/P7/8/8/8/8/8/K7 w - - 0 1", "a8=N") == "a7a8n", "a8=N underpromotes");
    check(parsed(start, "Nf4") == "none", "rejects illegal Nf4");
}

int main() {
    perftTests();
    searchTests();
    notationTests();
    if (failures) {
        std::cout << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << "all tests passed\n";
    return 0;
}
