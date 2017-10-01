if(NOT DEFINED ARM_TARGET)
  message(
    FATAL_ERROR
    "ARM toolchain detected. Please specify an ARM target via -DARM_TARGET={target}."
  )
endif()

set(ARM_ARCH_FILE "${RAPTOR_DIR}/build/cmake/arch/arm-${ARM_TARGET}.cmake")

if(EXISTS ${ARM_ARCH_FILE})
  include(${ARM_ARCH_FILE})
else()
  message(
    FATAL_ERROR
    "Unknown ARM target: ${ARM_TARGET}"
  )
endif()
