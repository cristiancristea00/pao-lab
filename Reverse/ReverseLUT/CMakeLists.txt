cmake_minimum_required(VERSION 3.27)
project(ReverseLUT)

set(CMAKE_CXX_STANDARD 23)


set(CUSTOM_FLAGS "-Wfatal-errors -Wpedantic -Wall -Wextra -Wconversion -Wshadow=local -Wdouble-promotion -Wformat=2 -Wformat-overflow=2             \
                  -Wformat-nonliteral -Wformat-security -Wformat-truncation=2 -Wnull-dereference -Wimplicit-fallthrough=3 -Wshift-overflow=2        \
                  -Wswitch-default -Wunused-parameter -Wunused-const-variable=2 -Wstrict-overflow=4 -Wstringop-overflow=3 -Wsuggest-attribute=pure  \
                  -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=malloc -Wsuggest-attribute=format   \
                  -Wmissing-format-attribute -Wsuggest-attribute=cold -Walloc-zero -Walloca -Wattribute-alias=2 -Wduplicated-branches -Wcast-qual")

set(CMAKE_CXX_FLAGS "${CUSTOM_FLAGS} -O0")
set(CMAKE_C_FLAGS "${CUSTOM_FLAGS} -O0")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -pipe -fno-builtin")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -march=native -pipe -fno-builtin")


add_executable(ReverseLUT main.cpp)
