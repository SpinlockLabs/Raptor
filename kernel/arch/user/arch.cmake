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

function(add_arch_file file)
  set(USER_ARCH_SRC ${USER_ARCH_SRC} ${file} PARENT_SCOPE)
endfunction()

function(add_arch_lib lib)
  set(USER_ARCH_LIBS ${USER_ARCH_LIBS} ${lib} PARENT_SCOPE)
endfunction()

if(WIN32)
  include(${CMAKE_CURRENT_LIST_DIR}/windows/arch.cmake)
else()
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/env-unix.c)
endif()

add_executable(kernel ${KERNEL_COMMON_SRC} ${USER_ARCH_SRC})
if(DEFINED USER_ARCH_LIBS)
  target_link_libraries(kernel ${USER_ARCH_LIBS})
endif()

if(ARCH_USER_HAS_POST_INIT)
  arch_user_post_init()
endif()
