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
    sed -E 's/([^ \/]*\.[^\.]+)\.o/\1\.bc/gm'
}

EMCC=${EMCC:-emcc}
ensure_dirs
IFS=$'\n' GLOBIGNORE='*' command eval 'CMDS=($(transform_compile_args))'
find -type f -name '*.bc' -exec rm {} ';'
for CMD in "${CMDS[@]}"
do
  ( ((i=i % 4)); ((i++==0)) && wait ) || true
  "${EMCC}" ${CMD} "${@}" ${EMCC_ARGS} 1>&2 &
done
wait

find -type f -name '*.bc'
