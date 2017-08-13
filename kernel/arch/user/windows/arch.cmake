set(WINDOWS_SRC
  env-windows.c
  net.c
)

foreach(F ${WINDOWS_SRC})
  add_arch_file(${CMAKE_CURRENT_LIST_DIR}/${F})
endforeach()

if(
  EXISTS "C:\\npcap-sdk" AND
  EXISTS "C:\\Windows\\System32\\Npcap\\Packet.dll" AND
  NOT DEFINED NO_NPCAP)
  set(NPCAP_COPY_DIR "${CMAKE_BUILD_TYPE}")
  if(NPCAP_COPY_DIR STREQUAL "")
    set(NPCAP_COPY_DIR "Debug")
  endif()

  add_definitions(-DUSE_NPCAP)
  add_arch_lib("C:\\npcap-sdk\\Lib\\x64\\Packet.lib")
  file(
    COPY "C:\\Windows\\System32\\Npcap\\Packet.dll"
    DESTINATION "${CMAKE_BINARY_DIR}\\${NPCAP_COPY_DIR}"
  )
endif()
