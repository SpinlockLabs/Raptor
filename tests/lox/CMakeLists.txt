add_library(lox-test lox_impl.c)
target_link_libraries(lox-test)

function(test NAME)
  set(ARGS ${ARGV})
  list(REMOVE_AT ARGS 0)
  add_executable(${NAME} ${ARGS})
  target_link_libraries(${NAME} lox dl lox-test)
  target_compile_options(${NAME} PRIVATE "-fno-builtin")
  target_include_directories(${NAME} PRIVATE ${RAPTOR_DIR})
  set(EXE $<TARGET_FILE:${NAME}>)

  add_test(
    NAME ${NAME}
    COMMAND ${EXE}
    WORKING_DIRECTORY ${TEST_DIR}
  )
endfunction()

test(test_list test_list.c)
