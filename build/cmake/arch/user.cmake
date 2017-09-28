if(NOT DEFINED DISABLE_USERSPACE)
  set(DISABLE_USERSPACE ON)
endif()

if(WEB)
  set(KERNEL_EXE_NAME "kernel.html")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  set(REAL_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
  set(REAL_ARCH "x86")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  if(CMAKE_C_FLAGS MATCHES ".*-m32.*")
    set(REAL_ARCH "x86")
  else()
    set(REAL_ARCH "x86_64")
  endif()
else()
  set(REAL_ARCH "user")
endif()

if(APPLE)
  set(REAL_ARCH "generic")
endif()

arch("user" "arch/user")

add_definitions(
  -DARCH_USER
)

if(CYGWIN)
  add_definitions(
    -DARCH_NO_SPINLOCK
  )

  target_link_libraries(kernel cygwin)
elseif(WEB)
  add_definitions(
    -DARCH_NO_SPINLOCK
  )

  kernel_cflags(
    -s ONLY_MY_CODE=1
    -s EXPORT_ALL=1
    -s LINKABLE=1
    -O1
  )
elseif(UNIX)
  target_link_libraries(kernel dl c)
elseif(WIN32)
  add_definitions(
    -DARCH_NO_SPINLOCK
  )

  if(MSVC)
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
  endif()
else()
  target_link_libraries(kernel dl c)
endif()

if(APPLE)
  ldflags(-e __start)
endif()

add_custom_target(run-kernel
  COMMAND $<TARGET_FILE:kernel>
  DEPENDS kernel
  USES_TERMINAL
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
