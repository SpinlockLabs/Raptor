#!/usr/bin/env bash
arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c kernel/arm/rpi/boot.S -o boot.o
arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel/arm/rpi/kernel.c -o kernel.o -O2 -Wall -Wextra
arm-none-eabi-gcc -T kernel/arm/rpi/linker.ld -o raptor.elf -ffreestanding -O2 -nostdlib boot.o kernel.o

