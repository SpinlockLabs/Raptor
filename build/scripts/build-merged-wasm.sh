#!/usr/bin/env bash
set -e

if [ -z ${1} ]
then
  echo "Usage: build-merged-wasm <target> [args]"
  exit 1
fi

TARGET=${1}
shift || true

"$(dirname ${0})/build-llvm-wasm.sh" "${@}" > /dev/null
"$(dirname ${0})/calculate-object-deps.sh" "${TARGET}" |
  sed 's/\.o$/\.wasm/gm' |
  xargs wasm-merge -o ${TARGET}.wasm
