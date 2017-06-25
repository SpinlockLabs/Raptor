raptor_add_arch("x86" "arch/x86")

raptor_set_arch_cflags(
        "x86"
        "-Wall \
        -std=gnu11 \
        -ffreestanding \
        -O2 \
        -m32 \
        -Wno-unused-command-line-argument"
)

raptor_set_arch_ldflags(
  "x86"
  "-nostartfiles \
  -Wno-unused-command-line-argument \
  -ffreestanding \
  -O2 \
  -T${KERNEL_DIR}/arch/x86/linker.ld \
  -m32"
)

if(BUILD_ARCH_x86)
    add_custom_target(qemu-x86
            COMMAND qemu-system-i386 -kernel ${CMAKE_BINARY_DIR}/raptor-x86.bin -cpu core2duo
            DEPENDS raptor-x86.bin)
endif()
