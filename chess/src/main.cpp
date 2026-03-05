#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Board.h"
#include "Engine.h"

int main() {
    Board board;
    Engine engine;
    board.reset();

    std::cout << "Mini chess engine (text-based).\n";
    std::cout << "Enter moves in algebraic form like e2e4. Type 'go' to let engine play.\n";
    board.print();

    std::string input;
    int searchDepth = 4;
    while (true) {
        if (board.whiteToMove) std::cout << "White> "; else std::cout << "Black> ";
        if (!std::getline(std::cin, input)) break;
        if (input == "quit" || input == "exit") break;
        if (input.rfind("setdepth",0) == 0) {
            int d = std::stoi(input.substr(8));
            if (d>0) searchDepth = d;
            std::cout << "depth set to " << searchDepth << "\n";
            continue;
        }
        if (input.rfind("go",0) == 0) {
            int d = searchDepth;
            auto parts = std::vector<std::string>();
            std::istringstream iss(input);
            std::string tok;
            iss >> tok; if (iss >> tok) d = std::stoi(tok);
            Move m = engine.findBestMove(board, d);
            std::cout << "engine plays " << m.toString() << "\n";
            board.makeMove(m);
            board.print();
            if (board.isThreefold()) {
                std::cout << "threefold repetition\n";
            }
            continue;
        }
        if (input.size() < 4) {
            std::cout << "invalid format\n";
            continue;
        }
        int from = (input[0]-'a') + (input[1]-'1')*8;
        int to = (input[2]-'a') + (input[3]-'1')*8;
        char prom = 0;
        if (input.size()>=5) prom = input[4];
        Move m(from,to,prom);
        auto moves = board.generateMoves();
        bool found=false;
        for (auto &mv: moves) {
            if (mv.from==from && mv.to==to && (prom==0||mv.promotion==prom)) {
                board.makeMove(mv);
                found=true;
                break;
            }
        }
        if (!found) {
            std::cout << "illegal move\n";
        } else {
            board.print();
            if (board.isThreefold()) {
                std::cout << "threefold repetition\n";
            }
        }
    }
    return 0;
}
