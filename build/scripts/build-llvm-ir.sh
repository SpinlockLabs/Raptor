#!/usr/bin/env bash
set -e

function collect_compile_args() {
  cat compile_commands.json |
    grep "\"command\"" |
    awk '{$1=""; $2=""; gsub("\",", ""); print}'
}

function ensure_dirs() {
  collect_compile_args |
    sed -Ern 's/(.*?\-o)(.*)\-c(.*)/\2/gpm' |
    xargs -n1 dirname |
    uniq |
    sort |
    xargs -n5 mkdir -p
}

function transform_compile_args() {
  collect_compile_args |
    sed -E 's/([^ \/]*\.[^\.]+)\.o/\1\.ll/gm'
}

CLANG=${CLANG:-clang}
ensure_dirs
IFS=$'\n' GLOBIGNORE='*' command eval 'CMDS=($(transform_compile_args))'
for CMD in "${CMDS[@]}"
do
  ${CLANG} -S ${CMD} "${@}" -emit-llvm ${CLANG_ARGS} 1>&2
done

find -type f -name '*.ll'
