cmake_minimum_required(VERSION 3.8)
project(raptor C)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include(${RAPTOR_DIR}/build/cmake/liblox.cmake)
include(${RAPTOR_DIR}/build/cmake/kernel-common.cmake)

include(${RAPTOR_DIR}/build/cmake/arch.cmake)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  if(NOT DEFINED ARM_TARGET)
    message(FATAL_ERROR "ARM toolchain detected. Please specify an ARM target via -DARM_TARGET={target}.")
  endif()
  include(${RAPTOR_DIR}/build/cmake/arch-arm-${ARM_TARGET}.cmake)
else()
  include(${RAPTOR_DIR}/build/cmake/arch-x86.cmake)
endif()

add_subdirectory(userspace)
