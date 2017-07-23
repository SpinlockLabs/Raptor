function(arch ARCH SRC_DIR)
  file(GLOB_RECURSE ARCH_SRC
    "${KERNEL_DIR}/${SRC_DIR}/*.c"
    "${KERNEL_DIR}/${SRC_DIR}/*.h"
    "${KERNEL_DIR}/${SRC_DIR}/*.s"
    "${KERNEL_DIR}/${SRC_DIR}/*.cpp"
    "${KERNEL_DIR}/${SRC_DIR}/*.hpp"
  )

  add_executable(kernel ${ARCH_SRC} ${KERNEL_COMMON_SRC})
  target_link_libraries(kernel gcc lox-kernel)
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
  set(KERNEL_LD_FLAGS "${KERNEL_LD_FLAGS} ${ARGLIST}" PARENT_SCOPE)
endfunction()

function(kernel_ldscript LDSCRIPT)
  set(KERNEL_LD_FLAGS "${KERNEL_LD_FLAGS} -T\"${LDSCRIPT}\"" PARENT_SCOPE)
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

kernel_cflags(
  -nostdlib
  -nostartfiles
  -ffreestanding
  -fno-lto
)

if(NOT CLANG)
  kernel_cflags(
    -fno-use-linker-plugin
  )
endif()

if(UBSAN)
  kernel_cflags(
    -fsanitize=undefined
  )
endif()
