cmake_minimum_required(VERSION 3.25)
project(Molybdenum)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static -static-libgcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

add_executable(Molybdenum main.cpp Position.cpp Position.h Constants.h UCI.cpp UCI.h
        Movegen.cpp
        Movegen.h
        Move.h
        BitStuff.h
        MagicBitboards.h
        Perft.h
        Utility.h
        search.cpp
        search.h
        eval.cpp
        eval.h
)
