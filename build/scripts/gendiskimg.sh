#!/usr/bin/env bash
set -e

if [ "${UID}" != "0" ]
then
  exec sudo "${BASH}" "${0}" "${UID}" "${@}"
fi

CUID="${1}"
shift

LOOP=$(losetup -f)
MKFS="mkfs.ext2"
SFDISK="sfdisk"
GRUB_INSTALL="grub-install"
GRUB2_INSTALL="grub2-install"

if ! which "${MKFS}" > /dev/null 2>&1
then
  MKFS="/usr/sbin/mkfs.ext2"
fi

if ! which "${SFDISK}" > /dev/null 2>&1
then
  SFDISK="/usr/sbin/sfdisk"
fi

if ! which "${GRUB_INSTALL}" > /dev/null 2>&1
then
  # Fedora prefixes grub commands with grub2
  if which "${GRUB2_INSTALL}" > /dev/null 2>&1
  then
    echo "Using grub2-install"
    GRUB_INSTALL="${GRUB2_INSTALL}"
  else
    GRUB_INSTALL="/usr/bin/grub-install"
  fi
fi

rm -rf raptor.img
dd if=/dev/zero of=raptor.img iflag=fullblock bs=1M count=100 > /dev/null 2>&1
chown ${CUID} raptor.img
echo -e 'label: dos\nstart=2048, type=83' | "${SFDISK}" raptor.img > /dev/null
"${SFDISK}" -A raptor.img 1 > /dev/null
losetup -P ${LOOP} raptor.img
"${MKFS}" ${LOOP}p1 > /dev/null

MDIR="$(mktemp -d)"
mount "${LOOP}p1" "${MDIR}"
cp -R filesystem/* "${MDIR}/"
"${GRUB_INSTALL}" --target=i386-pc -s --boot-directory="${MDIR}/boot" ${LOOP}
sync
umount "${MDIR}"
rm -rf "${MDIR}"
sync
losetup -d ${LOOP}
echo "Disk image created."
