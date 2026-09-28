# Mini Chess Engine

This repository contains a small text-based chess engine written in C++.  It
implements the full rules of chess, searches with negamax + alpha-beta and a
quiescence search, and lets you play against the engine or watch it play
itself from the command line.

## Features

* 64-square board with simple `enum` piece representation
* Fully legal move generation: check, castling (not out of or through check),
  en passant, promotions — verified with perft against published results
* Game end detection: checkmate, stalemate, threefold repetition, 50-move rule
* Evaluation: material + piece-square tables
* Search: negamax with alpha-beta pruning, iterative deepening, move ordering
  (best move first, MVV-LVA captures) and quiescence search, optional time limit
* FEN loading/saving

## Building

### Prerequisites

* A C++17-compatible compiler (`g++`, `clang++`, MSVC)
* CMake (optional, for cross-platform builds)

### Build steps

```powershell
# using g++ directly on Windows (MinGW/MSYS or WSL), from the project root:
cd "c:\orges programming\orges c++\chess"
g++ -std=c++17 -O2 src/*.cpp -o chess.exe

# or build via CMake:
mkdir build; cd build
cmake ..
cmake --build . --config Release
```

## Usage

```text
> .\chess.exe
Mini chess engine (text-based).
Enter moves like e4, Nf3 or O-O. Type 'go' to let the engine play, 'help' for all commands.
White> e4
you play 1. e4
Black> go
engine plays 1... e5
```

Moves use standard algebraic notation, the same as chess.com and lichess:
`K` king, `Q` queen, `R` rook, `B` bishop, `N` knight, and no letter for pawns.

| Move             | Meaning                                                 |
|------------------|---------------------------------------------------------|
| `e4`             | pawn to e4                                              |
| `Nf3`            | knight to f3                                            |
| `exd5`, `Bxc4`   | captures (the `x` is optional)                          |
| `O-O`, `O-O-O`   | castle king-side / queen-side (`0-0` works too)         |
| `e8=Q`           | promote (plain `e8` promotes to a queen)                |
| `Nbd2`, `R1e1`   | which piece, when two can reach the same square         |
| `e2e4`           | from-square/to-square also works                        |

| Command          | What it does                                            |
|------------------|---------------------------------------------------------|
| `go [depth]`     | engine plays a move for the side to move                |
| `auto [plies]`   | engine plays both sides until the game ends             |
| `undo`           | take back one move                                      |
| `moves`          | list legal moves                                        |
| `history`        | show the moves played so far                            |
| `setdepth N`     | default search depth (5)                                |
| `movetime MS`    | stop searching after MS milliseconds (0 = off)          |
| `fen [FEN]`      | show the current FEN, or load a position                |
| `new`            | start a new game                                        |
| `eval`           | static evaluation (+ = white is better)                 |
| `perft N`        | count leaf positions at depth N                         |
| `help`, `quit`   |                                                         |

## Tests

`tests/engine_tests.cpp` runs perft on six standard test positions (about 15
million positions in total, compared against the numbers on the
[Chess Programming Wiki](https://www.chessprogramming.org/Perft_Results)) plus
a few search and game-end checks.

```powershell
g++ -std=c++17 -O2 src/Board.cpp src/Engine.cpp src/Move.cpp tests/engine_tests.cpp -o engine_tests.exe
.\engine_tests.exe

# or with CMake, from the build directory:
ctest -C Release
```

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for
details.
