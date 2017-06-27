set(LIBLOX_SRC_DIR "${RAPTOR_DIR}/liblox")
file(GLOB_RECURSE LIBLOX_SRC
  "${LIBLOX_SRC_DIR}/*.c"
  "${LIBLOX_SRC_DIR}/*.h"
  "${LIBLOX_SRC_DIR}/*.s"
  "${LIBLOX_SRC_DIR}/*.cpp"
  "${LIBLOX_SRC_DIR}/*.hpp"
)

add_library(lox STATIC ${LIBLOX_SRC})
target_compile_options(lox PRIVATE "-fno-builtin")

add_library(lox-kernel STATIC ${LIBLOX_SRC})
