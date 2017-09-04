set(UNIX_SRC
  env-unix.c
)

foreach(F ${UNIX_SRC})
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/${F})
endforeach()

if(APPLE)
  include(${CMAKE_CURRENT_LIST_DIR}/../mac/arch.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/../linux/arch.cmake)
endif()
