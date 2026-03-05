#pragma once

#include <string>

struct Move {
    int from; // 0-63 square index
    int to;   // 0-63 square index
    char promotion; // 'q','r','b','n' or 0

    Move(int f, int t, char p = 0) : from(f), to(t), promotion(p) {}
    std::string toString() const;
};
