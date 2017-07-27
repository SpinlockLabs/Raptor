function(arch ARCH SRC_DIR)
  file(GLOB_RECURSE ARCH_SRC
    "${KERNEL_DIR}/${SRC_DIR}/*.c"
    "${KERNEL_DIR}/${SRC_DIR}/*.h"
    "${KERNEL_DIR}/${SRC_DIR}/*.s"
    "${KERNEL_DIR}/${SRC_DIR}/*.cpp"
    "${KERNEL_DIR}/${SRC_DIR}/*.hpp"
  )

  add_executable(kernel ${ARCH_SRC} ${KERNEL_COMMON_SRC})
  if(GCC)
    target_link_libraries(kernel gcc)
  endif()
  target_link_libraries(kernel lox-kernel)
  set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel.elf")
endfunction()

set(KERNEL_C_FLAGS "${CMAKE_C_FLAGS}")
set(KERNEL_ASM_FLAGS "${CMAKE_ASM_FLAGS}")
set(KERNEL_LD_FLAGS "${CMAKE_C_FLAGS}")

function(kernel_cflags)
  set(ARGLIST "")
  foreach(ARG ${ARGV})
    set(ARGLIST "${ARGLIST} ${ARG}")
  endforeach()
  set(KERNEL_C_FLAGS "${KERNEL_C_FLAGS} ${ARGLIST}" PARENT_SCOPE)
  set(KERNEL_ASM_FLAGS "${KERNEL_ASM_FLAGS} ${ARGLIST}" PARENT_SCOPE)

  if(NOT RAPTOR_CLANG_WIN)
    set(KERNEL_LD_FLAGS "${KERNEL_LD_FLAGS} ${ARGLIST}" PARENT_SCOPE)
  endif()
endfunction()

function(kernel_ldscript LDSCRIPT)
  set(KERNEL_LD_FLAGS "${KERNEL_LD_FLAGS} -Wl,-T\"${LDSCRIPT}\"" PARENT_SCOPE)
  set_source_files_properties(
    kernel/entry.c
    PROPERTIES
    OBJECT_DEPENDS "${LDSCRIPT}"
  )
endfunction()

function(arch_post_init)
  set_target_properties(
    kernel
    PROPERTIES
    LINK_FLAGS "${KERNEL_LD_FLAGS}"
    COMPILE_FLAGS "${KERNEL_C_FLAGS}"
  )

  set_target_properties(
    lox-kernel
    PROPERTIES
    COMPILE_FLAGS "${KERNEL_C_FLAGS}"
  )

  install(
    TARGETS kernel
    RUNTIME DESTINATION boot
  )
endfunction()

if(NOT COMPCERT)
  kernel_cflags(
    -nostdlib
    -nostartfiles
    -ffreestanding
    -fno-lto
  )
endif()

if(GCC)
  kernel_cflags(
    -fno-use-linker-plugin
  )
endif()

if(UBSAN)
  cflags(
    -D_UBSAN=1
    -fsanitize=undefined
  )
endif()
