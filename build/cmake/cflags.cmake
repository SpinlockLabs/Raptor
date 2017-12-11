option(ENABLE_LTO "Enable Link-Time Optimization" OFF)
option(ENABLE_WARN_ERRORS "Make Compiler Warnings Error" OFF)
option(ENABLE_WARN_EVERYTHING "Enable All Compiler Warnings" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

if(ENABLE_LTO)
  cflags(-flto)
endif()

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
    -fno-stack-protector
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

if(GCC)
  cflags(
    -Wduplicated-cond
    -Wlogical-op
    -Wduplicated-branches
    -Wnull-dereference
    -Wjump-misses-init
    -Wdouble-promotion
    -ffreestanding
  )
endif()

if(ENABLE_WARN_ERRORS)
  cflags(-Werror)
endif()

if(ENABLE_WARN_EVERYTHING)
  cflags(-Weverything)
endif()

if(ENABLE_ASAN)
  cflags(-fsanitize=address)
endif()

