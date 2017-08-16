#!/usr/bin/env bash
set -e

FILES=$("$(dirname ${0})/build-llvm-wast.sh" "${@}")
for F in ${FILES}
do
  A=$(echo $F | sed 's/\.wast$/\.wasm/')
  wasm-as ${F} -o ${A} 1>&2
done

find -type f -name '*.wasm'
