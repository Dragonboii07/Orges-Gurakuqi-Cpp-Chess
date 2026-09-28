#include "Engine.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>

static int pieceValue(Piece p) {
    switch (p) {
        case WP: case BP: return 100;
        case WN: case BN: return 320;
        case WB: case BB: return 330;
        case WR: case BR: return 500;
        case WQ: case BQ: return 900;
        case WK: case BK: return 20000;
        default: return 0;
    }
}

// best-first ordering makes alpha-beta cut far more branches:
// previous best move, then captures (most valuable victim, least valuable
// attacker first), then promotions, then quiet moves
void Engine::orderMoves(const Board& board, std::vector<Move>& moves, const Move* first) const {
    std::vector<std::pair<int, Move>> scored;
    scored.reserve(moves.size());
    for (const Move& m : moves) {
        int s = 0;
        if (first && m == *first) {
            s = 1000000;
        } else if (board.isCapture(m)) {
            Piece victim = board.squares[m.to];
            int victimValue = victim == EMPTY ? 100 : pieceValue(victim); // EMPTY = en passant
            s = 100000 + victimValue * 10 - pieceValue(board.squares[m.from]);
        }
        if (m.promotion == 'q') s += 90000;
        scored.emplace_back(s, m);
    }
    std::stable_sort(scored.begin(), scored.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });
    for (size_t i = 0; i < moves.size(); ++i) moves[i] = scored[i].second;
}

void Engine::checkTime() {
    // never abort the depth-1 search, so there is always a move to play
    if (timeLimitMs <= 0 || currentDepth <= 1) return;
    auto elapsed = std::chrono::steady_clock::now() - startTime;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= timeLimitMs) {
        stopped = true;
    }
}

// only searches captures and promotions, so the static evaluation is never taken
// in the middle of an exchange (the "horizon effect")
int Engine::quiescence(Board& board, int alpha, int beta) {
    if ((++nodes & 2047) == 0) checkTime();
    if (stopped) return 0;

    int standPat = board.whiteToMove ? board.evaluate() : -board.evaluate();
    if (standPat >= beta) return standPat;
    if (standPat > alpha) alpha = standPat;

    auto moves = board.generateMoves();
    moves.erase(std::remove_if(moves.begin(), moves.end(),
                               [&](const Move& m) { return !board.isCapture(m) && m.promotion != 'q'; }),
                moves.end());
    orderMoves(board, moves, nullptr);

    bool side = board.whiteToMove;
    for (const Move& m : moves) {
        board.makeMove(m);
        if (board.inCheck(side)) {
            board.unmakeMove(m);
            continue;
        }
        int score = -quiescence(board, -beta, -alpha);
        board.unmakeMove(m);
        if (stopped) return 0;
        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// negamax: every score is from the point of view of the side to move
int Engine::negamax(Board& board, int depth, int ply, int alpha, int beta) {
    if ((++nodes & 2047) == 0) checkTime();
    if (stopped) return 0;

    // a repeat inside the search is scored as a draw straight away
    if (board.isFiftyMove() || board.repetitionCount() >= 2) return 0;
    if (depth <= 0) return quiescence(board, alpha, beta);

    auto moves = board.generateMoves();
    orderMoves(board, moves, nullptr);

    bool side = board.whiteToMove;
    int best = -INF;
    int legal = 0;
    for (const Move& m : moves) {
        board.makeMove(m);
        if (board.inCheck(side)) {
            board.unmakeMove(m);
            continue;
        }
        ++legal;
        int score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
        board.unmakeMove(m);
        if (stopped) return 0;
        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }
    if (legal == 0) {
        // checkmate (prefer the quickest mate) or stalemate
        return board.inCheck(side) ? -MATE + ply : 0;
    }
    return best;
}

Move Engine::findBestMove(Board& board, int maxDepth) {
    nodes = 0;
    stopped = false;
    startTime = std::chrono::steady_clock::now();

    auto moves = board.generateLegalMoves();
    bestMove = moves.empty() ? Move(0,0) : moves[0];
    bestScore = 0;
    if (moves.size() <= 1) return bestMove;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        currentDepth = depth;
        orderMoves(board, moves, &bestMove);

        int alpha = -INF, beta = INF;
        Move iterBest = moves[0];
        int iterScore = -INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            int score = -negamax(board, depth - 1, 1, -beta, -alpha);
            board.unmakeMove(m);
            if (stopped) break;
            if (score > iterScore) {
                iterScore = score;
                iterBest = m;
            }
            if (score > alpha) alpha = score;
        }
        // a partial iteration may not have looked at the best move yet, so only
        // completed depths count
        if (stopped) break;
        bestMove = iterBest;
        bestScore = iterScore;

        if (verbose) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            std::cout << "  depth " << depth << "  score ";
            if (isMateScore(bestScore)) {
                int plies = MATE - std::abs(bestScore);
                std::cout << (bestScore > 0 ? "mate in " : "mated in ") << (plies + 1) / 2;
            } else {
                std::cout << bestScore << "cp";
            }
            std::cout << "  nodes " << nodes << "  time " << ms << "ms  best " << board.toSan(bestMove) << "\n";
        }
        if (isMateScore(bestScore)) break; // a deeper search can't find anything better
    }
    return bestMove;
}
