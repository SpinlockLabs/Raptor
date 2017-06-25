arch("x86" "arch/x86")

cflags(
  -nostartfiles
  -Wall
  -std=gnu11
  -ffreestanding
  -O2
  -m32
  -Wno-unused-command-line-argument
)

ldscript(${KERNEL_DIR}/arch/x86/linker.ld)

set(QEMU_FLAGS
  qemu-system-i386
    -kernel "${CMAKE_BINARY_DIR}/raptor.bin"
    -cpu core2duo
)

add_custom_target(qemu
  COMMAND ${QEMU_FLAGS}
  DEPENDS raptor.bin
)
