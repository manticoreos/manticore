cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(manticore)

set(CMAKE_C_FLAGS "-O3 -g -Wall -Wextra -Werror -Wno-unused-parameter")

add_subdirectory(libmanticore)
add_subdirectory(init)
