cmake_minimum_required(VERSION 3.10)
project(mini_chess_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# source files
file(GLOB SRC "src/*.cpp")
add_executable(chess ${SRC})

# installation (optional)
install(TARGETS chess RUNTIME DESTINATION bin)
