#!/usr/bin/env bash
set -e

function do_checksum() {
  sha256sum "${@}" | awk '{print $1}'
}

function fdlist() {
  find . | grep -v '^\.$' | sed -e 's/\.\///g' | sort
}

function folist() {
  find . -type f | grep -v '^\.$' | sed -e 's/\.\///g' | sort
}

function do_rpd_sum() {
  f="$(mktemp)"
  fdlist | do_checksum - >> "${f}"
  for p in $(folist)
  do
    do_checksum "${p}" >> "${f}"
  done
  do_checksum "${f}"
  rm "${f}"
}

if [ ! -z "${1}" ]
then
  cd "${1}"
fi

do_rpd_sum
