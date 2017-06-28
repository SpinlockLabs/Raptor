arch("x86" "arch/x86")
option(OPTIMIZE_NATIVE "Optimize for the native machine." OFF)

cflags(
  -m32
)

if(CLANG)
    cflags(-target i686-pc-elf)
endif()

if(OPTIMIZE_NATIVE)
    cflags(-march=native)
else()
    cflags(-mtune=generic)
endif()

kernel_ldscript(${KERNEL_DIR}/arch/x86/linker.ld)

set(QEMU_CMD_BASE
  qemu-system-i386
  -cpu core2duo
)

set(QEMU_CMD
  ${QEMU_CMD_BASE}
  -kernel "${CMAKE_BINARY_DIR}/kernel.elf"
)

add_custom_target(qemu
  COMMAND ${QEMU_CMD}
  DEPENDS kernel
)

add_custom_target(qemu-gdb
  COMMAND ${QEMU_CMD} -S -s -append debug
  DEPENDS kernel
)

add_custom_command(
  OUTPUT raptor.iso
  DEPENDS kernel
  COMMAND bash
            ${CMAKE_SOURCE_DIR}/build/scripts/mkgrubiso.sh
            "${CMAKE_BINARY_DIR}/kernel.elf"
  WORKING_DIRECTORY  "${CMAKE_BINARY_DIR}"
)

add_custom_target(iso
  DEPENDS raptor.iso
)

add_custom_target(qemu-iso
  COMMAND ${QEMU_CMD_BASE}
            -cdrom "${CMAKE_BINARY_DIR}/raptor.iso"
  DEPENDS iso
)

add_custom_target(bochs
  COMMAND bochs -q -f "${CMAKE_SOURCE_DIR}/build/bochs/raptor.bcfg"
  DEPENDS iso
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
