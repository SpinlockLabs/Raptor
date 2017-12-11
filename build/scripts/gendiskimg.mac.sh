#!/usr/bin/env bash
set -e

MKFS="/usr/local/opt/e2fsprogs/sbin/mkfs.ext2"
SFDISK="sfdisk"

rm -rf raptor.img
dd if=/dev/zero of=raptor.img bs=1m count=100 > /dev/null 2>&1
echo -e "y\nedit 1\n83\nn\n\n\nflag 1\nwrite\nexit\n" | fdisk -e raptor.img > /dev/null

DEV=$(hdiutil attach -nomount raptor.img | grep -E -o '\/dev\/disk(.)s1')
"${MKFS}" -F -F $DEV > /dev/null

MDIR="$(mktemp -d)"
ext2fuse $DEV $MDIR

hdiutil detach $DEV
