raptor_add_arch("arm-rpi" "kernel/arch/arm/rpi")

raptor_set_arch_cflags(
        "arm-rpi"
        "-Wall \
        -std=gnu11 \
        -mcpu=arm1176jzf-s \
        -ffreestanding \
        -O2 \
        -fpic \
        -Wno-unused-command-line-argument"
)

raptor_set_arch_ldflags(
        "arm-rpi"
        "-nostartfiles \
        -mcpu=arm1176jzf-s \
        -Wno-unused-command-line-argument \
        -ffreestanding \
        -O2 \
        -T${RAPTOR_DIR}/kernel/arch/arm/rpi/linker.ld"
)

if(BUILD_ARCH_arm-rpi)
    add_custom_target(qemu-arm-rpi
            COMMAND "qemu-system-arm"
              "-kernel" "${CMAKE_BINARY_DIR}/raptor-arm-rpi.bin"
              "-m" "256"
              "-M" "raspi2"
              "-serial" "stdio"
            DEPENDS raptor-arm-rpi.bin
    )
endif()
