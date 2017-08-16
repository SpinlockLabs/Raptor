#!/usr/bin/env bash
set -e

export LLC=${1:-llc}
shift || true

FILES=$("$(dirname ${0})/build-llvm-wasm.sh" ${LLC} "${@}")

emcc ${FILES} -s WASM=1
