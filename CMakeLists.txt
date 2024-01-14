cmake_minimum_required(VERSION 3.25)
project(Molybdenum)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -march=native")

add_executable(Molybdenum
        src/main.cpp
        src/Position.cpp
        src/UCI.cpp
        src/search.cpp
        src/timemanagment.cpp
        src/Transpositiontable.cpp
        src/Movepicker.h
        src/searchUtil.h
        src/UCIOptions.cpp
        src/UCIOptions.h
        src/Datagen/Datagen.cpp
        src/Datagen/Datagen.h
        src/bench.h
        src/nnue.cpp
        src/nnue.h
        src/incbin/incbin.h
        src/tune.h
        src/incbin/incbin.h
)
target_link_libraries(Molybdenum -static)
