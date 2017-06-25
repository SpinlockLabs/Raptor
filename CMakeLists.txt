cmake_minimum_required(VERSION 3.8)
project(raptor C)
option(OPTIMIZE_LTO "Enable Link-Time Optimization" OFF)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(build/cmake/common.cmake)

cflags(
  -Wall
  -std=gnu11
  -pipe
  -fno-builtin
  -nostartfiles
  -ffreestanding
)

if(CLANG)
  cflags(-Wno-unused-command-line-argument -O0)
  if(NOT OPTIMIZE_LTO)
    cflags(-fno-integrated-as)
  endif()
endif()

if(OPTIMIZE_LTO)
  cflags(-flto)
endif()

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${RAPTOR_DIR})

include(${RAPTOR_DIR}/build/cmake/liblox.cmake)
include(${RAPTOR_DIR}/build/cmake/kernel.cmake)
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
