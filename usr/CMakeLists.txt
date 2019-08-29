cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(manticore LANGUAGES C)

add_subdirectory(libmanticore)
add_subdirectory(liblinux)
add_subdirectory(tests)
