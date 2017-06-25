cmake_minimum_required(VERSION 3.8)
project(raptor-userspace C)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_subdirectory(bin)
