file(GLOB_RECURSE PROGRAM_LISTS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.program.cmake")

function(program EXE SRCFILES)
  set(SOURCE_LIST ${ARGV})
  list(REMOVE_AT SOURCE_LIST 0)
  set(SOURCE_FULL)
  foreach(SRC ${SOURCE_LIST})
    list(APPEND SOURCE_FULL "${CMAKE_CURRENT_LIST_DIR}/${SRC}")
  endforeach()
  add_executable(${EXE} ${SOURCE_FULL})
  add_dependencies(${EXE} lox)
  target_link_libraries(${EXE} lox crt0)
  target_include_directories(${EXE} PRIVATE ${RAPTOR_DIR})
  if(NOT COMPCERT)
    target_compile_options(${EXE} PRIVATE "-nostartfiles")
  endif()
  install(
    TARGETS ${EXE}
    RUNTIME DESTINATION bin
  )
endfunction()

foreach(PROGRAM_LIST ${PROGRAM_LISTS})
  include(${PROGRAM_LIST})
endforeach()
