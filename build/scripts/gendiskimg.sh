#!/usr/bin/env bash
set -e

MKFAT="mkfs.fat"

if ! which "${MKFAT}" > /dev/null 2>&1
then
  MKFAT="/usr/sbin/mkfs.fat"
fi

if [[ ! -f raptor.img ]]; then
  dd if=/dev/zero of=raptor.img iflag=fullblock bs=1M count=100
  "${MKFAT}" raptor.img
  sync
  echo "Disk image created."
fi
