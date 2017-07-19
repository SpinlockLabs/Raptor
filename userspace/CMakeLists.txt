cmake_minimum_required(VERSION 3.6)
project(raptor-userspace C)

enable_language(ASM)
add_subdirectory(bin)

include(internal/CMakeLists.txt)
