file(
  GLOB_RECURSE USER_ARCH_SRC
  "${CMAKE_CURRENT_LIST_DIR}/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/*.h"
)

foreach(F ${USER_ARCH_SRC})
  get_filename_component(ENAME "${F}" NAME_WE)
  if(ENAME MATCHES "env-.*")
    list(REMOVE_ITEM  USER_ARCH_SRC "${F}")
  endif()
endforeach()

if(WIN32)
  list(APPEND USER_ARCH_SRC "${CMAKE_CURRENT_LIST_DIR}/env-windows.c")
else()
  list(APPEND USER_ARCH_SRC "${CMAKE_CURRENT_LIST_DIR}/env-unix.c")
endif()

add_executable(kernel ${KERNEL_COMMON_SRC} ${USER_ARCH_SRC})
