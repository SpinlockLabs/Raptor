#!/usr/bin/env bash
set -e

if [ -z ${1} ]
then
  echo "Usage: calculate-object-deps <target>"
  exit 1
fi

$(dirname ${0})/calculate-deps.sh

if [ ! -f "depinfo/${1}/output" ]
then
  echo "ERROR: No such target '${1}'"
  exit 1
fi

cat depinfo/${1}/objects

for ARCHIVE in $(cat depinfo/${1}/archives)
do
  N=$(cat "depinfo/archives/${ARCHIVE}")
  cat depinfo/${N}/objects
done

