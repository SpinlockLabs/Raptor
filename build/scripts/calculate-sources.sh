#!/usr/bin/env bash
set -e

if [ -z ${1} ]
then
  echo "Usage: calculate-sources <target>"
  exit 1
fi

"$(dirname ${0})"/calculate-deps.sh

if [ ! -f "depinfo/${1}/output" ]
then
  echo "ERROR: No such target '${1}'"
  exit 1
fi

function obj2src() {
  cut -d"/" -f3- | sed 's/\.o$//g'
}

function get_only_target_src() {
  while read LINE
  do
    echo "${LINE}" | obj2src
  done < "depinfo/${1}/objects"
}

get_only_target_src "${1}"

for ARCHIVE in $(cat "depinfo/${1}/archives")
do
  N=$(cat "depinfo/archives/${ARCHIVE}")
  get_only_target_src "${N}"
done
