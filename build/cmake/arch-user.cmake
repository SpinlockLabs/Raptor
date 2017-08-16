if(WEB)
  set(KERNEL_EXE_NAME "kernel.html")
endif()

arch("user" "arch/user")

add_definitions(
  -DARCH_USER
  -DARCH_NO_SPINLOCK
)

if(WEB)
  kernel_cflags(
    -s ONLY_MY_CODE=1
  )
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
