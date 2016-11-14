cmake_minimum_required(VERSION 3.6)
project(pp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -std=c++14 -march=native -pthread")

add_executable(pp
    main.cpp
    stream/stream.h
    vectorization/vectorization.cpp
)

include_directories(./vectorization)