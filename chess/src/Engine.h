#pragma once

#include "Board.h"

struct Engine {
    int nodes;
    Move bestMove;

    Engine() : nodes(0), bestMove(0,0) {}

    int minimax(Board& board, int depth, int alpha, int beta);
    Move findBestMove(Board& board, int depth);
};
