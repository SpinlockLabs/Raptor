set(LINUX_SRC
  init.c
  init.h
)

foreach(F ${LINUX_SRC})
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/${F})
endforeach()
