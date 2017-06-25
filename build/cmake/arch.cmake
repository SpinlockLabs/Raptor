function(arch ARCH SRC_DIR)
  file(GLOB_RECURSE ARCH_SRC
    "${KERNEL_DIR}/${SRC_DIR}/*.c"
    "${KERNEL_DIR}/${SRC_DIR}/*.h"
    "${KERNEL_DIR}/${SRC_DIR}/*.s"
    "${KERNEL_DIR}/${SRC_DIR}/*.cpp"
    "${KERNEL_DIR}/${SRC_DIR}/*.hpp"
  )

  add_executable(raptor.bin ${ARCH_SRC} ${KERNEL_COMMON_SRC})
  target_link_libraries(raptor.bin lox-kernel)
endfunction()

function(ldscript LDSCRIPT)
  set_target_properties(raptor.bin PROPERTIES LINK_FLAGS "-T${LDSCRIPT}")
endfunction()
