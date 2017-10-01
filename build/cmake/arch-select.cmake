option(ARCH_USER "Enable Usermode Raptor OS" OFF)

if(NOT DEFINED ARCH_BUILD)
  set(ARCH_BUILD "${CMAKE_SYSTEM_PROCESSOR}")
endif()

if(ARCH_USER)
  include(${RAPTOR_DIR}/build/cmake/arch/user.cmake)
elseif(ARCH_BUILD STREQUAL "arm")
  include(${RAPTOR_DIR}/build/cmake/arch/arm.cmake)
elseif(ARCH_BUILD MATCHES "x86")
  include(${RAPTOR_DIR}/build/cmake/arch/x86.cmake)
elseif(ARCH_BUILD MATCHES "none")
  include(${RAPTOR_DIR}/build/cmake/arch/none.cmake)
else()
  message(
    FATAL_ERROR
    "Unknown Architecture: ${ARCH_BUILD}"
  )
endif()
