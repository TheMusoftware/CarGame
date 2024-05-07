cmake_minimum_required(VERSION 3.28)
project(CarGame)

set(CMAKE_CXX_STANDARD 98)

add_executable(CarGame main.cpp)
target_link_libraries(CarGame ncurses)