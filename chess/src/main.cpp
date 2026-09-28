#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Board.h"
#include "Engine.h"

static void printHelp() {
    std::cout <<
        "Moves (like chess.com):\n"
        "  e4, d5           pawn moves: just the square\n"
        "  Nf3, Bc4, Qd1    pieces: K=king Q=queen R=rook B=bishop N=knight\n"
        "  exd5, Nxe5       captures (the x is optional)\n"
        "  O-O, O-O-O       castle king-side / queen-side\n"
        "  e8=Q             promote (plain e8 promotes to a queen)\n"
        "  Nbd2, R1e1       say which piece when two can go to the same square\n"
        "  e2e4             from-square/to-square also works\n"
        "Commands:\n"
        "  go [depth]       engine plays a move for the side to move\n"
        "  auto [plies]     engine plays both sides until the game ends\n"
        "  undo             take back one move\n"
        "  moves            list legal moves\n"
        "  history          show the moves played so far\n"
        "  setdepth N       default search depth\n"
        "  movetime MS      stop searching after MS milliseconds (0 = off)\n"
        "  fen [FEN]        show the current FEN, or load a position\n"
        "  new              start a new game\n"
        "  eval             static evaluation (+ = white is better)\n"
        "  perft N          count leaf positions at depth N (move generator test)\n"
        "  help, quit\n";
}

// parse a whole-string integer; stoi alone accepts "4abc" and throws on "abc"
static bool parseInt(const std::string& s, int& out) {
    try {
        size_t used = 0;
        out = std::stoi(s, &used);
        return used == s.size();
    } catch (...) {
        return false;
    }
}

// a played move as it appears in the move list: "12. Nf3" or "12... Nf6"
struct PlayedMove {
    Move move;
    std::string san;
    int number;
    bool white;
    std::string label() const { return std::to_string(number) + (white ? ". " : "... ") + san; }
};

static PlayedMove playMove(Board& board, const Move& m, std::vector<PlayedMove>& played) {
    PlayedMove pm{m, board.toSan(m), board.fullmoveNumber, board.whiteToMove};
    board.makeMove(m);
    played.push_back(pm);
    return pm;
}

// one line per full move: "1. e4 e5"
static void printHistory(const std::vector<PlayedMove>& played) {
    if (played.empty()) {
        std::cout << "no moves yet\n";
        return;
    }
    for (size_t i = 0; i < played.size(); ++i) {
        const PlayedMove& pm = played[i];
        if (pm.white) std::cout << pm.number << ". " << pm.san;
        else if (i == 0) std::cout << pm.number << "... " << pm.san;
        else std::cout << ' ' << pm.san;
        if (!pm.white || i + 1 == played.size()) std::cout << '\n';
    }
}

// empty string while the game is still going
static std::string gameOverReason(Board& board) {
    if (board.generateLegalMoves().empty()) {
        if (board.inCheck(board.whiteToMove)) {
            return board.whiteToMove ? "checkmate - black wins" : "checkmate - white wins";
        }
        return "stalemate - draw";
    }
    if (board.isThreefold()) return "threefold repetition - draw";
    if (board.isFiftyMove()) return "fifty-move rule - draw";
    return "";
}

// prints the board and reports check or the end of the game; returns true if it's over
static bool afterMove(Board& board) {
    board.print();
    std::string reason = gameOverReason(board);
    if (!reason.empty()) {
        std::cout << reason << "\n";
        return true;
    }
    if (board.inCheck(board.whiteToMove)) std::cout << "check\n";
    return false;
}

int main() {
    Board board;
    Engine engine;
    std::vector<PlayedMove> played;

    std::cout << "Mini chess engine (text-based).\n";
    std::cout << "Enter moves like e4, Nf3 or O-O. Type 'go' to let the engine play, 'help' for all commands.\n";
    board.print();

    std::string line;
    int searchDepth = 5;
    while (true) {
        std::cout << (board.whiteToMove ? "White> " : "Black> ");
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string word, arg;
        if (!(iss >> word)) continue;
        // FEN and moves are case-sensitive, so only the command name is lowercased
        std::string rest;
        std::getline(iss >> std::ws, rest);
        std::istringstream(rest) >> arg;
        std::string cmd = word;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c) { return std::tolower(c); });

        if (cmd == "quit" || cmd == "exit") break;

        if (cmd == "help") {
            printHelp();
        } else if (cmd == "new") {
            board.reset();
            played.clear();
            board.print();
        } else if (cmd == "setdepth") {
            int d;
            if (parseInt(arg, d) && d > 0) {
                searchDepth = d;
                std::cout << "depth set to " << searchDepth << "\n";
            } else {
                std::cout << "usage: setdepth N (N > 0)\n";
            }
        } else if (cmd == "movetime") {
            int ms;
            if (parseInt(arg, ms) && ms >= 0) {
                engine.timeLimitMs = ms;
                std::cout << (ms ? "time limit " + std::to_string(ms) + "ms\n" : "time limit off\n");
            } else {
                std::cout << "usage: movetime MS (0 = off)\n";
            }
        } else if (cmd == "go" || cmd == "auto") {
            int d = searchDepth;
            int maxPlies = 1;
            if (cmd == "go" && !arg.empty() && !(parseInt(arg, d) && d > 0)) {
                std::cout << "usage: go [depth]\n";
                continue;
            }
            if (cmd == "auto") {
                maxPlies = 500;
                if (!arg.empty() && !(parseInt(arg, maxPlies) && maxPlies > 0)) {
                    std::cout << "usage: auto [plies]\n";
                    continue;
                }
            }
            std::string reason = gameOverReason(board);
            if (!reason.empty()) {
                std::cout << reason << "\n";
                continue;
            }
            for (int i = 0; i < maxPlies; ++i) {
                Move m = engine.findBestMove(board, d);
                std::cout << "engine plays " << playMove(board, m, played).label() << "\n";
                if (afterMove(board)) break;
            }
        } else if (cmd == "undo") {
            if (played.empty()) {
                std::cout << "nothing to undo\n";
            } else {
                std::cout << "took back " << played.back().label() << "\n";
                board.unmakeMove(played.back().move);
                played.pop_back();
                board.print();
            }
        } else if (cmd == "moves") {
            for (const Move& m : board.generateLegalMoves()) std::cout << board.toSan(m) << ' ';
            std::cout << "\n";
        } else if (cmd == "history") {
            printHistory(played);
        } else if (cmd == "fen") {
            if (rest.empty()) {
                std::cout << board.fen() << "\n";
            } else if (board.setFen(rest)) {
                played.clear();
                afterMove(board);
            } else {
                std::cout << "invalid FEN\n";
            }
        } else if (cmd == "eval") {
            std::cout << board.evaluate() << "cp\n";
        } else if (cmd == "perft") {
            int d;
            if (parseInt(arg, d) && d >= 0) {
                std::cout << "perft " << d << ": " << board.perft(d) << "\n";
            } else {
                std::cout << "usage: perft N\n";
            }
        } else {
            // moves are case-sensitive (bxc3 is a pawn, Bxc3 a bishop), so use the word as typed
            Move m(0, 0);
            if (board.parseMove(word, m)) {
                std::cout << "you play " << playMove(board, m, played).label() << "\n";
                afterMove(board);
            } else {
                std::cout << "'" << word << "' is not a legal move here (type 'moves' to see them, 'help' for commands)\n";
            }
        }
    }
    return 0;
}
