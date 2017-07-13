set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/filesystem")
set(FS_DIR "${CMAKE_SOURCE_DIR}/filesystem")

add_custom_target(
  filesystem
  COMMAND "${CMAKE_COMMAND}" -P "${RAPTOR_DIR}/build/cmake/fsclean.cmake"
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
  COMMAND "${CMAKE_COMMAND}"
    -D RAPTOR_DIR="${RAPTOR_DIR}"
    -P "${FS_DIR}/cmake/fsbuild.cmake"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
