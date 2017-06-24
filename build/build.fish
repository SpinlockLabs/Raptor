#!/usr/bin/env fish
#
# This file serves as a preliminary buildsystem. Will we ever adopt a real buildsystem? Not sure...
#
function build_arm
    mkdir -p out/arm
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c kernel/arm/rpi/boot.s -o out/arm/boot.o
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel/arm/rpi/entry.c -o out/arm/entry.o -O2 -Wall -Wextra
    arm-none-eabi-gcc -T kernel/arm/rpi/linker.ld -o out/arm/raptor.bin -ffreestanding -O2 -nostdlib out/arm/boot.o out/arm/entry.o
end

function build_x86
    mkdir -p out/x86
    set CFLAGS -std=gnu99 -ffreestanding -O2 -w -Ilibc/ -Lout/x86/libk/ -lk
    set LFLAGS -ffreestanding -O2 -nostdlib -Lout/x86/libk/ -lk
    build_libk_x86
    i686-elf-as kernel/x86/boot.s -o out/x86/boot.o
    i686-elf-as kernel/x86/isr.s -o out/x86/isr_asm.o
    i686-elf-as kernel/x86/irq.s -o out/x86/irq_asm.o
    i686-elf-gcc -c kernel/x86/entry.c -o out/x86/entry.o $CFLAGS
    i686-elf-gcc -c kernel/x86/vga.c -o out/x86/vga.o $CFLAGS
    i686-elf-gcc -c kernel/x86/gdt.c -o out/x86/gdt.o $CFLAGS
    i686-elf-gcc -c kernel/x86/idt.c -o out/x86/idt.o $CFLAGS
    i686-elf-gcc -c kernel/x86/isr.c -o out/x86/isr.o $CFLAGS
    i686-elf-gcc -c kernel/x86/io.c -o out/x86/io.o $CFLAGS
    i686-elf-gcc -c kernel/x86/irq.c -o out/x86/irq.o $CFLAGS
    i686-elf-gcc -c kernel/x86/timer.c -o out/x86/timer.o $CFLAGS
    i686-elf-gcc -c kernel/x86/keyboard.c -o out/x86/keyboard.o $CFLAGS
    i686-elf-gcc -T kernel/x86/linker.ld -o out/x86/raptor.bin out/x86/*.o $LFLAGS
end

function build_libk_x86
    mkdir -p out/x86/libk
    set CFLAGS -std=gnu99 -ffreestanding -O2 -w
    set LFLAGS -ffreestanding -O2 -nostdlib
    i686-elf-gcc -c libc/memset.c -o out/x86/libk/memset.o $CFLAGS
    i686-elf-gcc -c libc/strlen.c -o out/x86/libk/strlen.o $CFLAGS
    i686-elf-gcc -c libc/reverse.c -o out/x86/libk/reverse.o $CFLAGS
    i686-elf-gcc -c libc/itoa.c -o out/x86/libk/itoa.o $CFLAGS
    i686-elf-ar rcs out/x86/libk/libk.a out/x86/libk/*.o
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
