#!/usr/bin/env bash
set -e

export LLC=${1:-llc}

FILES=$($(dirname ${0})/build-llvm-as.sh --target=wasm32 -Oz)
for F in ${FILES}
do
  A=$(echo $F | sed 's/\.s$/\.wast/')
  s2wasm ${F} -o ${A}
done

find -type f -name '*.wasm'
