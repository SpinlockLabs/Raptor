arch("x86" "arch/x86")
option(ENABLE_X64 "Enable x86_64 mode." OFF)

set(OPTIMIZE_FOR "generic" CACHE STRING "CPU to Optimize For")
set(QEMU_CPU "core2duo" CACHE STRING "QEMU CPU")
set(QEMU_NIC "e1000" CACHE STRING "QEMU NIC Model")

cflags(
  -DARCH_X86
  -DARCH_HAS_SCHEDULER
)

if(ENABLE_X64 AND NOT COMPCERT)
  cflags(-m64)
elseif(NOT COMPCERT)
  cflags(-m32)
endif()

if(CLANG)
  set(
    CLANG_TARGET
    "i686-pc-linux-gnu"
    CACHE STRING
    "Clang Target"
  )

  cflags(
    -target "${CLANG_TARGET}"
    -mno-sse
  )
endif()

if(NOT COMPCERT)
  if(OPTIMIZE_FOR STREQUAL "generic")
    cflags(-mtune=${OPTIMIZE_FOR})
  else()
    cflags(-march=${OPTIMIZE_FOR})
  endif()

  cflags(
    -fno-stack-protector
    -fno-pic
  )
endif()

if(GCC)
  cflags(-fno-pie)
endif()

if(EXISTS ${RAPTOR_DIR}/kernel/arch/x86/acpi/include)
  kernel_cflags(-I${RAPTOR_DIR}/kernel/arch/x86/acpi/include)
endif()

kernel_ldscript("${KERNEL_DIR}/arch/x86/linker.ld")

set(QEMU_CMD_BASE
  qemu-system-i386
    -cpu ${QEMU_CPU}
    -m 1024
    -net nic,model=${QEMU_NIC}
    -drive "file=${CMAKE_BINARY_DIR}/raptor.img,format=raw,if=ide,media=disk"
)

set(QEMU_CMD
  ${QEMU_CMD_BASE}
)

add_custom_target(
  qemu
  COMMAND ${QEMU_CMD} -net user -serial file:kernel.log
  DEPENDS kernel diskimg
)

add_custom_target(
  qemu-cli
  COMMAND ${QEMU_CMD} -monitor none -nographic -net user
  DEPENDS kernel diskimg
)

add_custom_target(
  iso
  DEPENDS kernel filesystem
  COMMAND bash
            ${CMAKE_SOURCE_DIR}/build/scripts/mkgrubiso.sh
            "$<TARGET_FILE:kernel>"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_target(
  qemu-iso
  COMMAND ${QEMU_CMD_BASE}
            -cdrom "${CMAKE_BINARY_DIR}/raptor.iso"
            -boot d
            -monitor none
            -nographic
            -net user
  DEPENDS iso diskimg
)

add_custom_target(
  bochs
  COMMAND bochs -q -f "${CMAKE_SOURCE_DIR}/build/bochs/raptor.bcfg"
  DEPENDS iso kernel
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_target(
  diskimg
  COMMAND "${CMAKE_SOURCE_DIR}/build/scripts/gendiskimg.sh"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  DEPENDS filesystem
  SOURCES ${FS_DIR}
  USES_TERMINAL
)

if(RAPTOR_WINDOWS)
    add_custom_target(
      qemu-windows
      COMMAND "C:/Program Files/qemu/qemu-system-i386.exe"
            -netdev user,id=net0
            -device ${QEMU_NIC},netdev=net0
            -cpu ${QEMU_CPU}
            -m 1024
            -M q35
            -serial file:kernel.log
            -kernel "$<TARGET_FILE:kernel>"
      DEPENDS kernel
    )
endif()
