#!/usr/bin/env bash
set -e

ROOT="$(realpath $(dirname ${0})/../..)"
BIN="raptor.bin"

if [[ ! -z "${1}" ]]
then
  BIN="${1}"
fi

if [[ ! -f "${BIN}" ]]
then
  echo "ERROR: raptor.bin not found."
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

GRUB_ROOT="$(realpath $(dirname $(which grub-mkrescue))/..)"
cp -R "${ROOT}/build/grub" cdboot/boot/grub
cp "${BIN}" cdboot/boot/raptor.bin
grub-mkrescue -d "${GRUB_ROOT}/lib/grub/i386-pc" -o raptor.iso cdboot/
rm -rf cdboot
