#!/usr/bin/env bash
set -e

PART="${1}"

if [[ -z "${PART}" ]]
then
  echo "Usage: install-pi-sdcard <partition>"
  exit 1
fi

if [[ "${UID}" != "0" ]]
then
  exec sudo "${SHELL}" "${0}" "${@}"
fi

umount "${PART}" || true
umount "${PART}" || true
mkdir -p /tmp/sdcard
mount "${PART}" /tmp/sdcard
arm-none-eabi-objcopy kernel.elf -O binary /tmp/sdcard/kernel7.img
cp /tmp/sdcard/kernel7.img /tmp/sdcard/kernel.img
umount "${PART}"
sync
echo "Installed."
