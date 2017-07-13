if(NOT DEFINED RAPTOR_DIR)
  message(FATAL_ERROR "Raptor Directory was not defined.")
endif()

set(BUILD_DIR "")
set(USERSPACE_DIR "${RAPTOR_DIR}/userspace")
set(FS_DIR "${RAPTOR_DIR}/filesystem")
set(KERNEL_DIR "${RAPTOR_DIR}/kernel")
set(LOX_DIR "${RAPTOR_DIR}/liblox")
set(TESTS_DIR "${RAPTOR_DIR}/tests")

file(STRINGS "${LOX_DIR}/version.h" RAPTOR_VERSION_HEADER)

string(
  REGEX MATCH "\\\"(.+)\\\""
  RAPTOR_VERSION "${RAPTOR_VERSION_HEADER}"
)

string(
  REPLACE "\""
  ""
  RAPTOR_VERSION
  "${RAPTOR_VERSION}"
)

function(write PATH STR)
  file(WRITE "filesystem/${PATH}" "${STR}\n")
endfunction()

function(mkdir PATH)
  make_directory("filesystem/${PATH}")
endfunction()

file(GLOB_RECURSE FSBUILD_SCRIPTS "${FS_DIR}/cmake/*.cmake")
foreach(FSBUILD_SCRIPT ${FSBUILD_SCRIPTS})
  if(FSBUILD_SCRIPT STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
    continue()
  endif()
  include(${FSBUILD_SCRIPT})
endforeach()
