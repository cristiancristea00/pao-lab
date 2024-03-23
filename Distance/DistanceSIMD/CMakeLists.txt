cmake_minimum_required(VERSION 3.27)
project(DistanceSIMD)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_COLOR_DIAGNOSTICS ON)

set(DEBUG_FLAGS "-Wfatal-errors -Wpedantic -Wall -Wextra -Wconversion -Wshadow=local -Wdouble-promotion -Wformat=2 -Wformat-overflow=2             \
                 -Wformat-nonliteral -Wformat-security -Wformat-truncation=2 -Wnull-dereference -Wimplicit-fallthrough=3 -Wshift-overflow=2        \
                 -Wswitch-default -Wunused-parameter -Wunused-const-variable=2 -Wstrict-overflow=4 -Wstringop-overflow=3 -Wsuggest-attribute=pure  \
                 -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=malloc -Wsuggest-attribute=format   \
                 -Wmissing-format-attribute -Wsuggest-attribute=cold -Walloc-zero -Walloca -Wattribute-alias=2 -Wduplicated-branches -Wcast-qual")
                  
set(OPTIMIZED_FLAGS "-Ofast -march=native -pipe -fno-builtin -fopt-info-vec-optimized -ftree-vectorizer-verbose=6")


set(CMAKE_CXX_FLAGS "${OPTIMIZED_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_C_FLAGS "${OPTIMIZED_FLAGS} ${DEBUG_FLAGS}")

add_executable(DistanceSIMD main.cpp)

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries(DistanceSIMD PUBLIC OpenMP::OpenMP_CXX)
endif()