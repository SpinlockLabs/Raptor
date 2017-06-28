arch("arm-rpi" "arch/arm/rpi")

option(RPI_BOOT_PART "Raspberry Pi Boot Partition" "")

cflags(
  -mcpu=arm1176jzf-s
  -O2
  -fpic
)

kernel_ldscript("${KERNEL_DIR}/arch/arm/rpi/linker.ld")

set(QEMU_FLAGS
  qemu-system-arm
    -kernel "${CMAKE_BINARY_DIR}/kernel.elf"
    -m 256
    -M raspi2
    -serial stdio
)

add_custom_target(qemu
  COMMAND ${QEMU_FLAGS}
  DEPENDS raptor.bin
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
