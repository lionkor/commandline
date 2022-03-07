cmake_minimum_required(VERSION 3.13)

project(commandline)

if(UNIX)
    add_definitions(-Wall -Wextra -pedantic -std=c++17 -pthread)
endif(UNIX)
if(MSVC)
    add_definitions(/std:c++17 /permissive-)
endif(MSVC)

add_library(commandline commandline.cpp)

option(BUILD_EXAMPLES "Build example program" ON)

if(BUILD_EXAMPLES)
    add_executable(test main.cpp)
    if(UNIX)
        target_link_libraries(commandline pthread)
    endif(UNIX)

    target_link_libraries(test commandline)
endif()

