set(LIBFUEL_SRC_DIR "${RAPTOR_DIR}/libfuel")
file(GLOB_RECURSE LIBFUEL_SRC
  "${LIBFUEL_SRC_DIR}/*.c"
  "${LIBFUEL_SRC_DIR}/*.h"
  "${LIBFUEL_SRC_DIR}/*.s"
  "${LIBFUEL_SRC_DIR}/*.cpp"
  "${LIBFUEL_SRC_DIR}/*.hpp"
)

function(raptor_add_libfuel ARCH)
  set(KERNEL_NAME "raptor-${ARCH}.bin")
  add_library(fuel-${ARCH} STATIC ${LIBFUEL_SRC})
  add_dependencies(${KERNEL_NAME} fuel-${ARCH})
endfunction()
