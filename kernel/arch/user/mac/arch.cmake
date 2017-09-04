set(MAC_SRC
  init.c
  init.h
  ui.swift
)

foreach(F ${MAC_SRC})
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/${F})
endforeach()
