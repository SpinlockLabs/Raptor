set(WEB_SRC
  env-web.c
)

foreach(F ${WEB_SRC})
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/${F})
endforeach()
