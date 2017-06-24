cmake_minimum_required(VERSION 3.8)
project(raptor)

enable_language(ASM)

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include(${RAPTOR_DIR}/build/cmake/libfuel.cmake)

include(${RAPTOR_DIR}/build/cmake/arch.cmake)
file(GLOB ARCH_CMAKE_LISTS "${RAPTOR_DIR}/build/cmake/arch-*.cmake")

foreach(ARCH_CMAKE_LIST ${ARCH_CMAKE_LISTS})
  include(${ARCH_CMAKE_LIST})
endforeach()
