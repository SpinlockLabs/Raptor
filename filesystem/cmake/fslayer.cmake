file(
  GLOB_RECURSE
  FSLAYER_FILES
  RELATIVE "${FS_DIR}"
  "${FS_DIR}/*"
  EXCLUDE "cmake/*"
)

foreach(F ${FSLAYER_FILES})
  get_filename_component(D "${F}" DIRECTORY)
  if("${D}" MATCHES "^cmake")
    continue()
  endif()
  file(
    COPY "${FS_DIR}/${F}"
    DESTINATION "filesystem/${D}"
  )
endforeach()
