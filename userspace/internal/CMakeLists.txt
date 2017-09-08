set(CRT0_ARCH_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/arch/generic")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/arch/${LIBLOX_ARCH_DIR}")
  set(CRT0_ARCH_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/arch/${LIBLOX_ARCH_DIR}")
endif()

file(GLOB_RECURSE CRT0_SRC
  "${CRT0_ARCH_BASE_DIR}/*.c"
  "${CRT0_ARCH_BASE_DIR}/*.h"
  "${CRT0_ARCH_BASE_DIR}/*.s"
  "${CRT0_ARCH_BASE_DIR}/*.cpp"
  "${CRT0_ARCH_BASE_DIR}/*.hpp"
)

add_library(
  crt0
  STATIC
  "${CMAKE_CURRENT_LIST_DIR}/crt0.c"
  ${CRT0_SRC}
)

if(NOT COMPCERT)
  target_compile_options(
    crt0
    PRIVATE
    "-fno-builtin"
    "-fno-stack-protector"
  )
endif()

target_link_libraries(crt0 lox)
