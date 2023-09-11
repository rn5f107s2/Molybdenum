cmake_minimum_required(VERSION 3.25)
project(Molybdenum)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static -static-libgcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

add_executable(Molybdenum
        src/main.cpp
        src/Position.cpp
        src/UCI.cpp
        src/search.cpp
        src/eval.cpp
        src/timemanagment.cpp
        src/Transpositiontable.cpp
        src/Movepicker.h
        src/Datagen/datagen.cpp
        src/Datagen/datagen.h
)
