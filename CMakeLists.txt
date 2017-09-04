cmake_minimum_required(VERSION 3.5)
project(raptor C)

option(ARCH_USER "Enable Usermode Raptor OS" OFF)
option(ENABLE_LTO "Enable Link-Time Optimization" OFF)

enable_language(ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAPTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(build/cmake/common.cmake)
include(build/cmake/filesystem.cmake)

if(MSVC)
  cflags(
    /Wall
    /TC
  )
elseif(NOT COMPCERT)
  cflags(
    -Wall
    -Wextra
    -std=gnu11
    -pipe
  )
else()
  cflags(
    -Wall
    -fall
  )
endif()

if(CLANG)
  option(BUILD_LLVM_IR "Build LLVM IR" OFF)

  if(BUILD_LLVM_IR)
    set(ENABLE_LTO ON)
    cflags(-fuse-ld=gold -Wl,-plugin-opt=emit-llvm)
  endif()

  cflags(
    -Wno-unused-command-line-argument
  )
endif()

if(ENABLE_LTO)
  cflags(-flto)
endif()

include_directories(${RAPTOR_DIR})

include(${RAPTOR_DIR}/build/cmake/kernel.cmake)
include(${RAPTOR_DIR}/build/cmake/arch.cmake)

if(ARCH_USER)
  include(${RAPTOR_DIR}/build/cmake/arch-user.cmake)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  if(NOT DEFINED ARM_TARGET)
    message(FATAL_ERROR "ARM toolchain detected. Please specify an ARM target via -DARM_TARGET={target}.")
  endif()
  include(${RAPTOR_DIR}/build/cmake/arch-arm-${ARM_TARGET}.cmake)
else()
  include(${RAPTOR_DIR}/build/cmake/arch-x86.cmake)
endif()

include(${RAPTOR_DIR}/build/cmake/liblox.cmake)

arch_post_init()

add_subdirectory(userspace)

if(TESTS)
  include(${RAPTOR_DIR}/build/cmake/tests.cmake)
endif()

add_subdirectory(host)
