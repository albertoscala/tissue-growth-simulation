cmake_minimum_required(VERSION 3.16)

project(CellularAutomata
    VERSION 0.1
    LANGUAGES CXX
)

# -------------------------
# Global C++ settings
# -------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -------------------------
# Dependencies
# -------------------------
add_subdirectory(external/minifb)

# -------------------------
# Cellular Automata library
# -------------------------
add_library(cellular_automata_lib
    src/cellular_automata.cpp
)

target_include_directories(cellular_automata_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(cellular_automata_lib
    PUBLIC
        minifb
)

target_compile_options(cellular_automata_lib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# -------------------------
# Executable
# -------------------------
add_executable(cellular_automata
    src/main.cpp
)

target_link_libraries(cellular_automata
    PRIVATE
        cellular_automata_lib
)