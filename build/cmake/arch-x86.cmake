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

add_custom_target(iso
  COMMAND bash ${CMAKE_SOURCE_DIR}/build/scripts/mkgrubiso.sh "${CMAKE_BINARY_DIR}/raptor.bin"
  DEPENDS raptor.bin
  WORKING_DIRECTORY  "${CMAKE_BINARY_DIR}"
)

add_custom_target(bochs
  COMMAND bochs -q -f "${CMAKE_SOURCE_DIR}/build/bochs/raptor.bcfg"
  DEPENDS iso
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

