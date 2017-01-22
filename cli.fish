#!/usr/bin/env fish
function build_arm
    mkdir -p out/arm
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c kernel/arm/rpi/boot.s -o out/arm/boot.o
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel/arm/rpi/entry.c -o out/arm/entry.o -O2 -Wall -Wextra
    arm-none-eabi-gcc -T kernel/arm/rpi/linker.ld -o out/arm/raptor.bin -ffreestanding -O2 -nostdlib out/arm/boot.o out/arm/entry.o
end

function build_x86
    set CFLAGS -std=gnu99 -ffreestanding -O2 -w
    set LFLAGS -ffreestanding -O2 -nostdlib
    mkdir -p out/x86
    i686-elf-as kernel/x86/boot.s -o out/x86/boot.o
    i686-elf-as kernel/x86/isr.s -o out/x86/isr_asm.o
    i686-elf-gcc -c kernel/x86/entry.c -o out/x86/entry.o $CFLAGS
    i686-elf-gcc -c kernel/x86/tty.c -o out/x86/tty.o $CFLAGS
    i686-elf-gcc -c kernel/x86/gdt.c -o out/x86/gdt.o $CFLAGS
    i686-elf-gcc -c kernel/x86/idt.c -o out/x86/idt.o $CFLAGS
    i686-elf-gcc -c kernel/x86/isr.c -o out/x86/isr.o $CFLAGS
    i686-elf-gcc -T kernel/x86/linker.ld -o out/x86/raptor.bin $LFLAGS out/x86/boot.o out/x86/isr_asm.o out/x86/entry.o out/x86/tty.o out/x86/gdt.o out/x86/idt.o out/x86/isr.o
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

