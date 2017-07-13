#!/usr/bin/env bash
set -e

if [ ! -f raptor.img ]; then
  dd if=/dev/zero of=raptor.img iflag=fullblock bs=1M count=100
  /usr/sbin/mkfs.fat raptor.img
  sync
  echo "Disk image created."
fi
