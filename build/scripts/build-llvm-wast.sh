#!/usr/bin/env bash
set -e

export LLC=${1:-llc}
shift || true

FILES=$("$(dirname ${0})/build-llvm-as.sh" --target=wasm32 -Oz "${@}")
for F in ${FILES}
do
  A=$(echo $F | sed 's/\.s$/\.wast/')
  s2wasm ${F} -o ${A} 1>&2
done

find -type f -name '*.wast'
