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
  echo -e 'label: dos\nstart=2048, type=83' | sfdisk -a raptor.img
  sfdisk -A raptor.img 1
  sudo losetup -P /dev/loop0 raptor.img
  sudo "${MKFAT}" /dev/loop0p1
  sudo losetup -d /dev/loop0
  sync
  echo "Disk image created."
fi
