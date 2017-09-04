add_library(crt0 STATIC internal/crt0.c)

if(NOT COMPCERT)
  target_compile_options(crt0 PRIVATE "-fno-builtin")
endif()

target_link_libraries(crt0 lox)
