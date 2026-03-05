# Mini Chess Engine

This repository contains a tiny text-based chess engine written in C++.  It
supports basic move generation (including castling, en-passant, and promotions),
an evaluation function with piece-square tables, and a recursive minimax search
with alpha-beta pruning and iterative deepening.  The command-line interface
lets you play against the engine or have it play itself.

## Features

* 64-square board with simple `enum` piece representation
* Move generation for all pieces plus special rules
* State tracking: castling rights, en-passant, threefold repetition
* Evaluation: material + basic positional tables
* Search: minimax with alpha-beta and iterative deepening
* CLI commands: `e2e4`, `go`, `setdepth N`, etc.

## Building

### Prerequisites

* A C++17-compatible compiler (`g++`, `clang++`, MSVC)
* CMake (optional, for cross-platform builds)

### Build steps

```powershell
# using g++ directly on Windows (MinGW/MSYS or WSL):
cd "c:\orges programming\orges c++\chess\src"
g++ -std=c++17 -O2 *.cpp -o chess.exe

# or build via CMake from project root:
cd "c:\orges programming\orges c++\chess"
mkdir build && cd build
cmake ..
cmake --build .
```

The executable will be produced in `src/` (when using g++) or in `build/` when
using CMake.

## Usage

Run the engine and enter moves:

```text
> .\src\chess.exe
Mini chess engine (text-based).
Enter moves in algebraic form like e2e4. Type 'go' to let engine play.
...
White> e2e4
Black> go
```

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for
details.
