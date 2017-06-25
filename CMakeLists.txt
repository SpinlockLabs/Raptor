cmake_minimum_required(VERSION 3.8)
project(raptor C)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include(${RAPTOR_DIR}/build/cmake/liblox.cmake)
include(${RAPTOR_DIR}/build/cmake/kernel-common.cmake)

include(${RAPTOR_DIR}/build/cmake/arch.cmake)
file(GLOB ARCH_CMAKE_LISTS "${RAPTOR_DIR}/build/cmake/arch-*.cmake")

foreach(ARCH_CMAKE_LIST ${ARCH_CMAKE_LISTS})
  include(${ARCH_CMAKE_LIST})
endforeach()
