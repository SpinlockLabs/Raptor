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

if ! which "${MKFS}" > /dev/null 2>&1
then
  MKFS="/usr/sbin/mkfs.ext2"
fi

if ! which "${SFDISK}" > /dev/null 2>&1
then
  SFDISK="/usr/sbin/sfdisk"
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
grub-install --target=i386-pc -s --boot-directory="${MDIR}/boot" ${LOOP}
sync
umount "${MDIR}"
rm -rf "${MDIR}"
sync
losetup -d ${LOOP}
echo "Disk image created."
