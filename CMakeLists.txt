cmake_minimum_required(VERSION 3.1)

project(commandline)

set(CMAKE_CXX_STANDARD 11)

if(UNIX)
    add_definitions(-Wall -Wextra -pedantic)
endif(UNIX)
if(MSVC)
    add_definitions(/permissive-)
endif(MSVC)

include(FindThreads)

add_library(commandline commandline.cpp)
target_link_libraries(commandline Threads::Threads)
target_include_directories(commandline PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

option(BUILD_EXAMPLES "Build example program" ON)

if(BUILD_EXAMPLES)
    add_executable(test main.cpp)
    target_link_libraries(test commandline)
endif()

