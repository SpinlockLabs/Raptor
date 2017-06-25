function(raptor_set_arch ARCH SRC_DIR)
  file(GLOB_RECURSE ARCH_SRC
    "${KERNEL_DIR}/${SRC_DIR}/*.c"
    "${KERNEL_DIR}/${SRC_DIR}/*.h"
    "${KERNEL_DIR}/${SRC_DIR}/*.s"
    "${KERNEL_DIR}/${SRC_DIR}/*.cpp"
    "${KERNEL_DIR}/${SRC_DIR}/*.hpp"
  )

  add_executable(raptor.bin ${ARCH_SRC} ${KERNEL_COMMON_SRC})
  add_dependencies(raptor.bin lox-kernel)
  target_link_libraries(raptor.bin lox-kernel)
  target_include_directories(raptor.bin BEFORE PRIVATE ${RAPTOR_DIR})
endfunction()

function(raptor_set_arch_cflags CFLAGS_TO_ADD)
  set_target_properties(raptor.bin PROPERTIES COMPILE_FLAGS ${CFLAGS_TO_ADD})
  set_target_properties(lox-kernel PROPERTIES COMPILE_FLAGS ${CFLAGS_TO_ADD})
endfunction()

function(raptor_set_arch_ldflags LDFLAGS_TO_ADD)
  set_target_properties(raptor.bin PROPERTIES LINK_FLAGS ${LDFLAGS_TO_ADD})
  set_target_properties(lox-kernel PROPERTIES LINK_FLAGS ${LDFLAGS_TO_ADD})
endfunction()
