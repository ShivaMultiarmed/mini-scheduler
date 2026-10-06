cmake_minimum_required(VERSION 4.3)
project(mini_scheduler)

set(CMAKE_CXX_STANDARD 20)

add_executable(mini_scheduler main.cpp
        Pool.cpp
        Pool.h
        Worker.cpp
        Worker.h)
