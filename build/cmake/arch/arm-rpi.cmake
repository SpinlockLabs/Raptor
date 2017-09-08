arch("arm-rpi" "arch/arm/rpi")
arch_include_src("${KERNEL_DIR}/arch/arm/common")

option(RPI_BOOT_PART "Raspberry Pi Boot Partition" "")

cflags(
  -march=armv6zk
  -mcpu=arm1176jzf-s
  -mfpu=vfp
  -mfloat-abi=hard
  --specs=nosys.specs
  -O2
  -fpic
)

if(CLANG)
  cflags(
    -target arm-none-eabi
  )
elseif(GCC)
  target_link_libraries(kernel gcc)
endif()

add_definitions(
  -DARCH_ARM
  -DARCH_ARM_RPI
  -DARCH_NO_SPINLOCK
)

kernel_ldscript("${KERNEL_DIR}/arch/arm/rpi/linker.ld")

set(QEMU_CMD
  qemu-system-arm
    -kernel "$<TARGET_FILE:kernel>"
    -m 1024
    -M raspi2
    -serial stdio
)

add_custom_target(qemu
  COMMAND ${QEMU_CMD}
  DEPENDS kernel
)

add_custom_target(qemu-cli
  COMMAND ${QEMU_CMD} -nographic -monitor none
  DEPENDS kernel
)

if(NOT RPI_BOOT_PART STREQUAL "")
  set(INSTALL_CMD
    bash
      "${CMAKE_SOURCE_DIR}/build/scripts/install-pi-sdcard.sh"
      "${RPI_BOOT_PART}"
  )

  add_custom_target(install-pi-sdcard
    COMMAND ${INSTALL_CMD}
    DEPENDS kernel
  )
endif()

if(NOT RPI_TTY STREQUAL "")
  add_custom_target(update-live
    COMMAND bash "${CMAKE_SOURCE_DIR}/build/scripts/rpi-live-update.sh" ${RPI_TTY}
    DEPENDS kernel
  )
endif()
