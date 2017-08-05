#!/usr/bin/env bash
set -e

PI_TTY="${1}"

if [[ -z "${PI_TTY}" ]]
then
  echo "Usage: rpi-live-update <tty>"
  exit 1
fi

if [[ "${UID}" != "0" ]]
then
  exec sudo "${SHELL}" "${0}" "${@}"
fi

arm-none-eabi-objcopy kernel.elf -O binary kernel.img

function uint32() {
  printf '0: %.8x' ${1} |
    sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' |
    xxd -r -g0
}

stty -F "${PI_TTY}" raw 115200

/bin/echo -e -n '\x11' > "${PI_TTY}"
uint32 $(stat -c %s kernel.img) > "${PI_TTY}"
sleep 0.1
pv kernel.img > "${PI_TTY}"
