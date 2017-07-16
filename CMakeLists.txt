cmake_minimum_required(VERSION 3.8)
project(raptor C)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(build/cmake/common.cmake)
include(build/cmake/filesystem.cmake)

cflags(
  -Wall
  -Wextra
  -std=gnu11
  -pipe
)

if(CMAKEBUILD_TYPE STREQUAL "Debug")
  cflags(-g)
endif()

if(CLANG)
  cflags(-Wno-unused-command-line-argument -O0)
  cflags(-fno-integrated-as)
endif()

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
arch_post_init()

add_subdirectory(userspace)

if(TESTS)
  include(${RAPTOR_DIR}/build/cmake/tests.cmake)
endif()
