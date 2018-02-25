#!/usr/bin/env bash
set -e

ROOT=$(realpath "$(dirname ${0})/../..")
BIN="kernel.elf"
GRUB_MKRESCUE="grub-mkrescue"
GRUB2_MKRESCUE="grub2-mkrescue"
GRUB_RESCUE=""

if [[ ! -z "${1}" ]]
then
  BIN="${1}"
fi

if [[ ! -f "${BIN}" ]]
then
  echo "ERROR: kernel.elf not found."
  exit 1
fi

if ! which "${GRUB_MKRESCUE}" > /dev/null 2>&1
then
  if which "${GRUB2_MKRESCUE}" > /dev/null 2>&1
  then
    GRUB_RESCUE="$(which ${GRUB2_MKRESCUE})"
  fi
else
  GRUB_RESCUE="$(which ${GRUB_MKRESCUE})"
fi

if [ GRUB_RESCUE = "" ]
then
  echo "ERROR: GRUB tools are not installed."
  exit 1
fi

if [[ -d cdboot ]]
then
  rm -rf cdboot
fi

mkdir -p cdboot/boot/grub

GRUB_RESCUE_DIR="$(dirname ${GRUB_RESCUE})"
GRUB_ROOT="$(realpath ${GRUB_RESCUE_DIR}/..)"
cp -R kernel.elf cdboot/boot/kernel.elf
cp -R "${ROOT}/filesystem/boot/grub/grub.cfg" cdboot/boot/grub/grub.cfg
"${GRUB_RESCUE}" -d "${GRUB_ROOT}/lib/grub/i386-pc" -o raptor.iso cdboot/
rm -rf cdboot
