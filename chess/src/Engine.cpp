#include "Engine.h"
#include <limits>

int Engine::minimax(Board& board, int depth, int alpha, int beta) {
    nodes++;
    if (board.isThreefold()) {
        return 0; // draw
    }
    if (depth == 0) {
        return board.evaluate();
    }
    auto moves = board.generateMoves();
    if (moves.empty()) {
        // checkmate/stalemate detection simple
        return board.whiteToMove ? -999999 : 999999;
    }
    bool maximizing = board.whiteToMove;
    int best = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    for (auto& m : moves) {
        Piece captured = board.squares[m.to];
        bool prevWhite = board.whiteToMove;
        unsigned char prevCast = board.castlingRights;
        int prevEP = board.enPassant;
        board.makeMove(m);
        int val = minimax(board, depth-1, alpha, beta);
        board.unmakeMove(m, captured, prevWhite, prevCast, prevEP);
        if (maximizing) {
            if (val > best) best = val;
            if (val > alpha) alpha = val;
        } else {
            if (val < best) best = val;
            if (val < beta) beta = val;
        }
        if (alpha >= beta) break;
    }
    return best;
}

// helper - simple timer
#include <chrono>
Move Engine::findBestMove(Board& board, int maxDepth) {
    nodes = 0;
    bestMove = Move(0,0);
    bool maximizing = board.whiteToMove;
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();
    int bestVal = maximizing ? alpha : beta;

    // iterative deepening
    for (int depth = 1; depth <= maxDepth; ++depth) {
        alpha = std::numeric_limits<int>::min();
        beta = std::numeric_limits<int>::max();
        for (auto& m : board.generateMoves()) {
            Piece captured = board.squares[m.to];
            bool prevWhite = board.whiteToMove;
            unsigned char prevCast = board.castlingRights;
            int prevEP = board.enPassant;
            board.makeMove(m);
            int val = minimax(board, depth-1, alpha, beta);
            board.unmakeMove(m, captured, prevWhite, prevCast, prevEP);
            if (maximizing) {
                if (val > bestVal) {
                    bestVal = val;
                    bestMove = m;
                }
                if (val > alpha) alpha = val;
            } else {
                if (val < bestVal) {
                    bestVal = val;
                    bestMove = m;
                }
                if (val < beta) beta = val;
            }
            if (alpha >= beta) break;
        }
    }
    return bestMove;
}
