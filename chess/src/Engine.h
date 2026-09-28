#pragma once

#include <chrono>
#include <vector>
#include "Board.h"

struct Engine {
    static constexpr int INF = 1000000;
    static constexpr int MATE = 100000;   // score for mate at the root; minus ply further down

    long long nodes;
    Move bestMove;
    int bestScore;        // from the side to move's point of view
    int timeLimitMs;      // 0 = search to full depth regardless of time
    bool verbose;         // print a line per completed depth

    Engine() : nodes(0), bestMove(0,0), bestScore(0), timeLimitMs(0), verbose(true) {}

    Move findBestMove(Board& board, int maxDepth);
    int negamax(Board& board, int depth, int ply, int alpha, int beta);
    int quiescence(Board& board, int alpha, int beta);

    static bool isMateScore(int score) { return score > MATE - 1000 || score < -MATE + 1000; }

private:
    std::chrono::steady_clock::time_point startTime;
    int currentDepth = 0;
    bool stopped = false;

    void orderMoves(const Board& board, std::vector<Move>& moves, const Move* first) const;
    void checkTime();
};
