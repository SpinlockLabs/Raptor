#!/usr/bin/env bash
set -e

cd "$(dirname ${0})/../.."

cmdir=""

if [ -d cmake-build-debug ]
then
  cmdir="cmake-build-debug"
elif [ -d cmake-build-release ]
then
  cmdir="cmake-build-release"
elif [ -d out ]
then
  cmdir="out"
else
  cmdir="${1}"
fi

if [ -z ${cmdir} ]
then
  echo "Usage: run-clang-tidy <builddir>"
  exit 1
fi

if [ ! -f ${cmdir}/compile_commands.json ]
then
  echo "ERROR: Unable to find compile_commands.json"
  exit 1
fi

FILES=$(
  cat ${cmdir}/compile_commands.json |
  grep '"file":' |
  awk '{gsub("\"", ""); print $2}'
)
exec clang-check -analyze -p=${cmdir} ${FILES}
