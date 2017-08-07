function(arch ARCH SRC_DIR)
  set(ARCH "${ARCH}" PARENT_SCOPE)

  set(ARCH_SCRIPT "${KERNEL_DIR}/${SRC_DIR}/arch.cmake")
  if(EXISTS "${ARCH_SCRIPT}")
    include("${ARCH_SCRIPT}")
  else()
    file(GLOB_RECURSE ARCH_SRC
      "${KERNEL_DIR}/${SRC_DIR}/*.c"
      "${KERNEL_DIR}/${SRC_DIR}/*.h"
      "${KERNEL_DIR}/${SRC_DIR}/*.s"
      "${KERNEL_DIR}/${SRC_DIR}/*.cpp"
      "${KERNEL_DIR}/${SRC_DIR}/*.hpp"
    )

    add_executable(kernel ${ARCH_SRC} ${KERNEL_COMMON_SRC})
  endif()

  if(GCC)
    target_link_libraries(kernel gcc)
  endif()
  target_link_libraries(kernel lox-kernel)
  set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel.elf")
endfunction()

function(arch_include_src DIR)
  file(GLOB_RECURSE EXTRA_SRC
    "${DIR}/*.c"
    "${DIR}/*.h"
    "${DIR}/*.s"
    "${DIR}/*.cpp"
    "${DIR}/*.hpp"
  )

  target_sources(kernel PUBLIC ${EXTRA_SRC})
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

kernel_cflags(
  -nostdlib
  -nostartfiles
)

if(NOT COMPCERT)
  kernel_cflags(
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
