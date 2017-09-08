cmake_minimum_required(VERSION 3.5)
project(raptor C)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(build/cmake/common.cmake)
include(build/cmake/filesystem.cmake)
include(build/cmake/cflags.cmake)

include_directories(${RAPTOR_DIR})

include(${RAPTOR_DIR}/build/cmake/kernel.cmake)
include(${RAPTOR_DIR}/build/cmake/arch.cmake)
include(${RAPTOR_DIR}/build/cmake/arch-select.cmake)
include(${RAPTOR_DIR}/build/cmake/liblox.cmake)

arch_post_init()

add_subdirectory(userspace)

option(TESTS "Enable Tests" OFF)

if(TESTS)
  include(${RAPTOR_DIR}/build/cmake/tests.cmake)
endif()

add_subdirectory(host)
