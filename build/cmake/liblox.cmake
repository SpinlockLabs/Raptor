set(LIBLOX_SRC_DIR "${RAPTOR_DIR}/liblox")

set(LIBLOX_SRC "")
file(GLOB LIBLOX_DIRS
  RELATIVE ${LIBLOX_SRC_DIR}
  "${LIBLOX_SRC_DIR}/*"
)

set(LIBLOX_ARCH_DIR "generic")

if(EXISTS "${LIBLOX_SRC_DIR}/arch/${ARCH}")
  set(LIBLOX_ARCH_DIR "${ARCH}")
endif()

foreach(DIR ${LIBLOX_DIRS})
  if(DIR MATCHES ".*\\..*")
    list(APPEND LIBLOX_SRC "${LIBLOX_SRC_DIR}/${DIR}")
    continue()
  endif()
  set(SEARCH "${LIBLOX_SRC_DIR}/${DIR}")
  if(DIR STREQUAL "arch")
    set(SEARCH "${LIBLOX_SRC_DIR}/arch/${LIBLOX_ARCH_DIR}")
  endif()

  file(GLOB_RECURSE SRC
    "${SEARCH}/*.c"
    "${SEARCH}/*.h"
    "${SEARCH}/*.s"
    "${SEARCH}/*.cpp"
    "${SEARCH}/*.hpp"
  )

  list(APPEND LIBLOX_SRC ${SRC})
endforeach()

foreach(F ${LIBLOX_SRC})
  file(RELATIVE_PATH FREL "${LIBLOX_SRC_DIR}" "${F}")
  get_filename_component(BN "${FREL}" NAME_WE)
  get_filename_component(BD "${FREL}" DIRECTORY)
  set(FEQ "${LIBLOX_SRC_DIR}/arch/${LIBLOX_ARCH_DIR}/${FREL}")
  set(FAS "${LIBLOX_SRC_DIR}/arch/${LIBLOX_ARCH_DIR}/${BD}/${BN}.s")
  if(EXISTS "${FEQ}" OR EXISTS "${FAS}")
    list(REMOVE_ITEM LIBLOX_SRC ${F})
  endif()
endforeach()

add_library(lox STATIC ${LIBLOX_SRC})
if(NOT COMPCERT)
  target_compile_options(lox PRIVATE "-fno-builtin")
endif()
add_library(lox-kernel STATIC ${LIBLOX_SRC})
