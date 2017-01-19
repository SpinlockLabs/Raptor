#!/usr/bin/env bash
qemu-system-arm -kernel raptor.elf -m 256 -M raspi2 -serial stdio

