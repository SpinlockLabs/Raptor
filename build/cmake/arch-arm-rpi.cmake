raptor_set_arch("arm-rpi" "arch/arm/rpi")

raptor_set_arch_cflags(
        "-Wall \
        -std=gnu11 \
        -mcpu=arm1176jzf-s \
        -ffreestanding \
        -O2 \
        -fpic \
        -Wno-unused-command-line-argument"
)

raptor_set_arch_ldflags(
        "-nostartfiles \
        -mcpu=arm1176jzf-s \
        -Wno-unused-command-line-argument \
        -ffreestanding \
        -O2 \
        -T${KERNEL_DIR}/arch/arm/rpi/linker.ld"
)

add_custom_target(qemu-arm-rpi
        COMMAND "qemu-system-arm"
          "-kernel" "${CMAKE_BINARY_DIR}/raptor.bin"
          "-m" "256"
          "-M" "raspi2"
          "-serial" "stdio"
        DEPENDS raptor.bin
)
