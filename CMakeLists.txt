cmake_minimum_required(VERSION 3.13)

project(commandline)

set(CMAKE_CXX_COMPILER clang++)

add_definitions(-Wall -Wextra -pedantic -std=c++17)

add_executable(test main.cpp)

add_library(commandline commandline.cpp)

target_link_libraries(test commandline pthread)
