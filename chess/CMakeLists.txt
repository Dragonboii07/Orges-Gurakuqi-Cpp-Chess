cmake_minimum_required(VERSION 3.10)
project(mini_chess_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# search speed depends heavily on optimisation, so default to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# engine code shared by the game and the tests
add_library(chess_core STATIC src/Board.cpp src/Engine.cpp src/Move.cpp)
target_include_directories(chess_core PUBLIC src)

add_executable(chess src/main.cpp)
target_link_libraries(chess PRIVATE chess_core)

enable_testing()
add_executable(engine_tests tests/engine_tests.cpp)
target_link_libraries(engine_tests PRIVATE chess_core)
add_test(NAME engine_tests COMMAND engine_tests)

# installation (optional)
install(TARGETS chess RUNTIME DESTINATION bin)
