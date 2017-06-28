#!/usr/bin/env bash
set -e

ROOT=$(realpath "$(dirname ${0})/../..")
BIN="kernel.elf"

if [[ ! -z "${1}" ]]
then
  BIN="${1}"
fi

if [[ ! -f "${BIN}" ]]
then
  echo "ERROR: kernel.elf not found."
  exit 1
fi

if ! which grub-mkrescue >/dev/null 2>&1
then
  echo "ERROR: GRUB tools are not installed."
  exit 1
fi

if [[ -d cdboot ]]
then
  rm -rf cdboot
fi

mkdir -p cdboot/boot

GRUB_RESCUE="$(which grub-mkrescue)"
GRUB_RESCUE_DIR="$(dirname ${GRUB_RESCUE})"
GRUB_ROOT="$(realpath ${GRUB_RESCUE_DIR}/..)"
cp -R "${ROOT}/build/grub" cdboot/boot/grub
cp "${BIN}" cdboot/boot/kernel.elf
grub-mkrescue -d "${GRUB_ROOT}/lib/grub/i386-pc" -o raptor.iso cdboot/
rm -rf cdboot
