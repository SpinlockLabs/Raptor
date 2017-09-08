option(ENABLE_LTO "Enable Link-Time Optimization" OFF)

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
