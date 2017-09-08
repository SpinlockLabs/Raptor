option(ARCH_USER "Enable Usermode Raptor OS" OFF)

set(ARCH_BUILD "${CMAKE_SYSTEM_PROCESSOR}")

if(ARCH_USER)
  include(${RAPTOR_DIR}/build/cmake/arch/user.cmake)
elseif(ARCH_BUILD STREQUAL "arm")
  if(NOT DEFINED ARM_TARGET)
    message(FATAL_ERROR "ARM toolchain detected. Please specify an ARM target via -DARM_TARGET={target}.")
  endif()
  include(${RAPTOR_DIR}/build/cmake/arch/arm-${ARM_TARGET}.cmake)
else()
  include(${RAPTOR_DIR}/build/cmake/arch/x86.cmake)
endif()
