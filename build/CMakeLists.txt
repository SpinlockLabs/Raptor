cmake_minimum_required(VERSION 3.8)
project(raptor)

enable_language(ASM)

get_filename_component(RAPTOR_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
include(cmake/libfuel.cmake)

include(cmake/arch.cmake)
file(GLOB ARCH_CMAKE_LISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/arch-*.cmake")

foreach(ARCH_CMAKE_LIST ${ARCH_CMAKE_LISTS})
  include(${ARCH_CMAKE_LIST})
endforeach()
