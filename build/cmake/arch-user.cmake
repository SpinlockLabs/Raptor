arch("user" "arch/user")

add_definitions(
  -DARCH_USER
  -DARCH_NO_SPINLOCK
)

if(CYGWIN)
  target_link_libraries(kernel cygwin)
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
endif()
