if(WEB)
  set(KERNEL_EXE_NAME "kernel.html")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  set(REAL_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
  set(REAL_ARCH "x86")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set(REAL_ARCH "x86_64")
else()
  set(REAL_ARCH "user")
endif()

arch("user" "arch/user")

add_definitions(
  -DARCH_USER
  -DARCH_NO_SPINLOCK
)

if(CYGWIN)
  target_link_libraries(kernel cygwin)
elseif(WEB)
  kernel_cflags(
    -s ONLY_MY_CODE=1
    -s EXPORT_ALL=1
    -s LINKABLE=1
    -O1
  )
elseif(UNIX)
  target_link_libraries(kernel dl c)
elseif(WIN32)
  kernel_cflags(
    /ZW:nostdlib
    /MT
    /nodefaultlib
    /GS-
    /Oi-
  )

  string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

  ldflags(
    /FORCE:MULTIPLE
  )
else()
  target_link_libraries(kernel dl c)
endif()
