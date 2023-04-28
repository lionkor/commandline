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

set(COMMANDLINE_PLATFORM_LINUX 0)
set(COMMANDLINE_PLATFORM_WINDOWS 0)

if (WIN32)
    set(COMMANDLINE_PLATFORM_WINDOWS 1)
elseif (LINUX OR APPLE OR CYGWIN OR UNIX) # just check a bunch even if they may repeat
    set(COMMANDLINE_PLATFORM_LINUX 1)
endif()

add_library(commandline commandline.cpp impls.h windows_impl.cpp linux_impl.cpp)
target_link_libraries(commandline Threads::Threads)
if (${COMMANDLINE_PLATFORM_WINDOWS})
    target_compile_definitions(commandline PRIVATE -DPLATFORM_WINDOWS=1)
elseif (${COMMANDLINE_PLATFORM_LINUX})
    target_compile_definitions(commandline PRIVATE -DPLATFORM_LINUX=1)
endif()
target_include_directories(commandline PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

option(BUILD_EXAMPLES "Build example program" ON)

if(BUILD_EXAMPLES)
    add_executable(commandline_test main.cpp)
    target_link_libraries(commandline_test commandline)
    if (${COMMANDLINE_PLATFORM_WINDOWS})
        target_compile_definitions(commandline PRIVATE -DPLATFORM_WINDOWS=1)
    elseif (${COMMANDLINE_PLATFORM_LINUX})
        target_compile_definitions(commandline PRIVATE -DPLATFORM_LINUX=1)
    endif()
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT commandline_test)
endif()

