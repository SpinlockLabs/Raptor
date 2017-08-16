arch("user" "arch/user")

add_definitions(
  -DARCH_USER
  -DARCH_NO_SPINLOCK
)

if(NOT WIN32)
  target_link_libraries(kernel dl c)
else()
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
