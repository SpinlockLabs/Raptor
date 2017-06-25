arch("x86" "arch/x86")
option(OPTIMIZE_NATIVE "Optimize for the native machine." OFF)

cflags(
  -nostartfiles
  -Wall
  -std=gnu11
  -ffreestanding
  -m32
  -Wno-unused-command-line-argument
)

if(CLANG)
    cflags(-target i686-pc-elf)
endif()

if(OPTIMIZE_NATIVE)
    cflags(-march=native)
else()
    cflags(-mtune=generic)
endif()

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
