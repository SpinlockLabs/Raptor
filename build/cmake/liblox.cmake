set(LIBLOX_SRC_DIR "${RAPTOR_DIR}/liblox")
file(GLOB_RECURSE LIBLOX_SRC
  "${LIBLOX_SRC_DIR}/*.c"
  "${LIBLOX_SRC_DIR}/*.h"
  "${LIBLOX_SRC_DIR}/*.s"
  "${LIBLOX_SRC_DIR}/*.cpp"
  "${LIBLOX_SRC_DIR}/*.hpp"
)

function(raptor_add_liblox ARCH)
  set(KERNEL_NAME "raptor-${ARCH}.bin")
  add_library(lox-${ARCH} STATIC ${LIBLOX_SRC})
  add_dependencies(${KERNEL_NAME} lox-${ARCH})
  target_link_libraries(${KERNEL_NAME} lox-${ARCH})
  target_include_directories(${KERNEL_NAME} BEFORE PRIVATE ${RAPTOR_DIR}/liblox)
endfunction()
