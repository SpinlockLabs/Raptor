#!/usr/bin/env fish
function build_arm
    mkdir -p out/arm
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c kernel/arm/rpi/boot.s -o out/arm/boot.o
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel/arm/rpi/entry.c -o out/arm/entry.o -O2 -Wall -Wextra
    arm-none-eabi-gcc -T kernel/arm/rpi/linker.ld -o out/arm/raptor.bin -ffreestanding -O2 -nostdlib out/arm/boot.o out/arm/entry.o
end

function build_x86
    mkdir -p out/x86
    i686-elf-as kernel/x86/boot.s -o out/x86/boot.o
    # Getting rid of all warnings, I don't care right now.
    i686-elf-gcc -c kernel/x86/entry.c -o out/x86/entry.o -std=gnu99 -ffreestanding -O2 -w # -Wall -Wextra
    i686-elf-gcc -c kernel/x86/tty.c -o out/x86/tty.o -std=gnu99 -ffreestanding -O2 -w # -Wall -Wextra
    i686-elf-gcc -c kernel/x86/gdt.c -o out/x86/gdt.o -std=gnu99 -ffreestanding -O2 -w # -Wall -Wextra
    i686-elf-gcc -c kernel/x86/idt.c -o out/x86/idt.o -std=gnu99 -ffreestanding -O2 -w # -Wall -Wextra
    i686-elf-gcc -T kernel/x86/linker.ld -o out/x86/raptor.bin -ffreestanding -O2 -nostdlib out/x86/boot.o out/x86/entry.o out/x86/tty.o out/x86/gdt.o out/x86/idt.o
end

function qemu_arm
    qemu-system-arm -kernel out/arm/raptor.bin -m 256 -M raspi2 -serial stdio
end

function qemu_x86
    qemu-system-i386 -cpu core2duo -kernel out/x86/raptor.bin
end

function clean
    rm -rf out/
end

#build_arm
build_x86
