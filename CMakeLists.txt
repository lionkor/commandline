cmake_minimum_required(VERSION 3.10)

project(commandline)

set(CMAKE_CXX_STANDARD 11)

if (UNIX)
    set(COMMON_WARNINGS -Wall -Wextra -pedantic -Werror)
elseif (MSVC)
    set(COMMON_WARNINGS /permissive- /W3 /WX)
endif ()

include(FindThreads)

set(COMMANDLINE_PLATFORM_LINUX 0)
set(COMMANDLINE_PLATFORM_WINDOWS 0)

if (WIN32)
    set(COMMANDLINE_PLATFORM_WINDOWS 1)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif (LINUX OR APPLE OR CYGWIN OR UNIX)
    set(COMMANDLINE_PLATFORM_LINUX 1)
endif ()

if (DEFINED COMMANDLINE_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${COMMANDLINE_SHARED_LIBS}")
endif ()

set(COMMANDLINE_SRCS
    src/impls.h
    src/windows_impl.cpp
    src/linux_impl.cpp
    src/backends/InteractiveBackend.cpp
    src/backends/InteractiveBackend.h
    src/backends/Backend.cpp
    src/backends/Backend.h
    src/commandline.h
    src/commandline.cpp
    src/backends/BufferedBackend.cpp
    src/backends/BufferedBackend.h
    src/helper/ansi.h
    src/helper/ansi.cpp
)

add_library(commandline
    ${COMMANDLINE_LIBTYPE}
    ${COMMANDLINE_SRCS}
)

add_library(commandline::commandline ALIAS commandline)

if (COMMON_WARNINGS)
    target_compile_options(commandline PRIVATE ${COMMON_WARNINGS})
endif ()

target_compile_definitions(commandline PRIVATE DOCTEST_CONFIG_DISABLE=1)
target_link_libraries(commandline PRIVATE Threads::Threads)

if (COMMANDLINE_PLATFORM_WINDOWS)
    target_compile_definitions(commandline PRIVATE PLATFORM_WINDOWS=1)
elseif (COMMANDLINE_PLATFORM_LINUX)
    target_compile_definitions(commandline PRIVATE PLATFORM_LINUX=1)
endif ()

target_include_directories(commandline
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)

option(BUILD_EXAMPLES "Build example program" ON)

if (BUILD_EXAMPLES)
    add_executable(commandline_test src/main.cpp)
    target_link_libraries(commandline_test PRIVATE commandline)
    if (COMMANDLINE_PLATFORM_WINDOWS)
        target_compile_definitions(commandline_test PRIVATE PLATFORM_WINDOWS=1)
    elseif (COMMANDLINE_PLATFORM_LINUX)
        target_compile_definitions(commandline_test PRIVATE PLATFORM_LINUX=1)
    endif ()
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT commandline_test)
endif ()

option(COMMANDLINE_TESTS "Build tests" ON)

if (COMMANDLINE_TESTS)
    add_executable(commandline_tests tests/tests.cpp)
    target_link_libraries(commandline_tests PRIVATE commandline)
    if (COMMANDLINE_PLATFORM_WINDOWS)
        target_compile_definitions(commandline_tests PRIVATE PLATFORM_WINDOWS=1)
    elseif (COMMANDLINE_PLATFORM_LINUX)
        target_compile_definitions(commandline_tests PRIVATE PLATFORM_LINUX=1)
    endif ()
    enable_testing()
    add_test(NAME commandline_tests COMMAND commandline_tests)
endif ()
