raptor_set_arch("x86" "arch/x86")

raptor_set_arch_cflags(
        "-Wall \
        -std=gnu11 \
        -ffreestanding \
        -O2 \
        -m32 \
        -Wno-unused-command-line-argument"
)

raptor_set_arch_ldflags(
  "-nostartfiles \
  -Wno-unused-command-line-argument \
  -ffreestanding \
  -O2 \
  -T${KERNEL_DIR}/arch/x86/linker.ld \
  -m32"
)

add_custom_target(qemu-x86
        COMMAND qemu-system-i386 -kernel ${CMAKE_BINARY_DIR}/raptor.bin -cpu core2duo
        DEPENDS raptor.bin)
