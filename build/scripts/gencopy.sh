#!/usr/bin/env bash
set -e

if [ -z ${1} ] || [ -z ${2} ]
then
  echo "Usage: gencopy <extension> <directory>"
  exit 1
fi

P='*.'"${1}"

mkdir -p ${2}

FF="$(realpath ${2})"

for X in $(find . -type f -name "${P}")
do
  if realpath ${X} | grep "${FF}" > /dev/null
  then
    continue
  fi
  OD="$(echo ${X} | sed 's/\.\/CMakeFiles\///' | sed 's/\.dir//')"
  T="${2}/${OD}"
  N="$(dirname ${T})"
  mkdir -p "${N}"
  echo "${X} => ${T}"
  cp "${X}" "${T}"
done
