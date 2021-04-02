cmake_minimum_required(VERSION 3.13)

project(commandline)

if(UNIX)
    add_definitions(-Wall -Wextra -pedantic -std=c++17 -pthread)
endif(UNIX)
if(MSVC)
    add_definitions(/std:c++17 /permissive- /MD /WX /nologo /EHsc)
endif(MSVC)

add_executable(test main.cpp)

add_library(commandline commandline.cpp)

if(UNIX)
    target_link_libraries(commandline pthread)
endif(UNIX)

target_link_libraries(test commandline)

