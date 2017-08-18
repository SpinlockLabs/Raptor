#!/usr/bin/env bash
set -e

if [ -z ${1} ]
then
  echo "Usage: build-asmjs <target> [args]"
  exit 1
fi

TARGET=${1}
shift || true

export EMCC=${EMCC:-emcc}

"$(dirname ${0})/build-emcc.sh" > /dev/null
FILES=$(
  "$(dirname ${0})/calculate-object-deps.sh" "${TARGET}" |
      sed 's/\.o$/\.bc/gm'
)

${EMCC} \
  -s ONLY_MY_CODE=1 \
  -s LINKABLE=1 \
  -s EXPORT_ALL=1 \
  -O1 \
  -o ${TARGET}.html \
  ${FILES}
