file(GLOB_RECURSE SRC
  "${CMAKE_CURRENT_LIST_DIR}/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/*.h"
  "${CMAKE_CURRENT_LIST_DIR}/*.s"
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
)

add_library(
  uspi
  STATIC
  ${SRC}
)

target_include_directories(
  uspi
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
)

target_include_directories(
  kernel
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${RAPTOR_DIR}
)

target_link_libraries(kernel uspi)
target_link_libraries(uspi lox-kernel)
