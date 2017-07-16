#!/usr/bin/env bash
set -e

MKFS="mkfs.ext2"
SFDISK="sfdisk"

if ! which "${MKFS}" > /dev/null 2>&1
then
  MKFS="/usr/sbin/mkfs.ext2"
fi

if ! which "${SFDISK}" > /dev/null 2>&1
then
  SFDISK="/usr/sbin/sfdisk"
fi

if [[ ! -f raptor.img ]]; then
  dd if=/dev/zero of=raptor.img iflag=fullblock bs=1M count=100
  "${MKFS}" raptor.img
  echo -e 'label: dos\nstart=2048, type=83' | "${SFDISK}" -a raptor.img
  "${SFDISK}" -A raptor.img 1
  sudo losetup -P /dev/loop0 raptor.img
  sudo "${MKFS}" /dev/loop0p1
  sudo losetup -d /dev/loop0
  sync
  echo "Disk image created."
fi
