cmake_minimum_required(VERSION 3.16)

project(CellularAutomata
    VERSION 0.1
    LANGUAGES CXX
)

# -------------------------
# MiniFB
# -------------------------
add_subdirectory(dependencies/minifb)

# -------------------------
# Your executable
# -------------------------
add_executable(cellular_automata
    main.cpp
    cellular_automata.cpp
)

# Make project root available for includes like <cellular_automata.hpp>
target_include_directories(cellular_automata PRIVATE ${CMAKE_SOURCE_DIR})

target_compile_features(cellular_automata PRIVATE cxx_std_20)

target_compile_options(cellular_automata PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# -------------------------
# Link MiniFB
# -------------------------
target_link_libraries(cellular_automata PRIVATE minifb)