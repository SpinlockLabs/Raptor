#!/usr/bin/env bash
set -e

FILES=$($(dirname ${0})/build-llvm-ir.sh "${@}")
for F in ${FILES}
do
  A=$(echo $F | sed 's/\.ll$/\.s/')
  ${LLC:-llc} -asm-verbose=false -o ${A} ${F} 1>&2
done

find -type f -name '*.s'
