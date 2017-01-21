#!/usr/bin/env fish
function build_arm
    mkdir -p out/arm
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c kernel/arm/rpi/boot.s -o out/arm/boot.o
    arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel/arm/rpi/entry.c -o out/arm/entry.o -O2 -Wall -Wextra
    arm-none-eabi-gcc -T kernel/arm/rpi/linker.ld -o out/arm/raptor.elf -ffreestanding -O2 -nostdlib out/arm/boot.o out/arm/entry.o
end

function build_x86
    mkdir -p out/x86
    i686-elf-as kernel/x86/boot.s -o out/x86/boot.o
    i686-elf-gcc -c kernel/x86/entry.c -o out/x86/entry.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
    i686-elf-gcc -T kernel/x86/linker.ld -o out/x86/raptor.elf -ffreestanding -O2 -nostdlib out/x86/boot.o out/x86/entry.o
end

function clean
    rm -rf out/
end

#build_arm
build_x86

