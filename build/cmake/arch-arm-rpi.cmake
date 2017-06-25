arch("arm-rpi" "arch/arm/rpi")

cflags(
  -mcpu=arm1176jzf-s
  -ffreestanding
  -O2
  -fpic
  -Wno-unused-command-line-argument
)

ldflags(
  "-T${KERNEL_DIR}/arch/arm/rpi/linker.ld"
)

set(QEMU_FLAGS
  qemu-system-arm
    -kernel "${CMAKE_BINARY_DIR}/raptor.bin"
    -m 256
    -M raspi2
    -serial stdio
)

add_custom_target(qemu
  COMMAND ${QEMU_FLAGS}
  DEPENDS raptor.bin
)
