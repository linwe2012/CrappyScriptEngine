cmake_minimum_required(VERSION 3.13)
project(whocares)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic-errors -std=c++1z -lstdc++fs")
link_libraries(stdc++fs)
include_directories(.)

add_executable(whocares
        attribute.h
        bootstrapper.cc
        bootstrapper.h
        builtins.cc
        builtins.h
        checks.cc
        checks.h
        compile-debugger.cc
        compile-debugger.h
        console.cc
        console.h
        factory.cc
        factory.h
        game.h
        listeners.h
        parser-ast.cc
        parser-ast.h
        pretty_print.h
        templates.h
        templates_library.h
        token.h
        whocares.cc
        whocares.h
        world.cc
        world.h
        worldgen.cc
        worldgen.h
        wrapper.h "macro-utils.h")
