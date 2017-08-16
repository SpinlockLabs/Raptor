#!/usr/bin/env bash
set -e

OUTDIR="${OUTDIR:-depinfo}"

function write() {
  mkdir -p "${OUTDIR}/$(dirname ${1})"
  cat > "${OUTDIR}/${1}"
}

LINKS=$(find -type f -name 'link.txt')

declare -A LIBS
TARGETS=()

for LINK in ${LINKS}
do
  NAME=$(echo "${LINK}" |
    tr '/' '\n' |
    grep "\.dir$" |
    sed 's/\.dir//g'
  )

  TARGETS+=(${NAME})

  cat "${LINK}" |
    tr ' ' '\n' |
    grep -A1 '^\-o$' |
    tail -n1 |
    write ${NAME}/output

  cat "${LINK}" |
    tr ' ' '\n' |
    grep "\.o$" |
    write ${NAME}/objects

  LIB=""
  if grep "ranlib" "${LINK}" > /dev/null
  then
    LIB=$(
      grep "ranlib" "${LINK}" |
      tr ' ' '\n' |
      grep "\.a$"
    )

    LIBS["${NAME}"]="${LIB}"
  fi

  cat "${LINK}" |
    tr ' ' '\n' |
    grep "\.a$" |
    grep -v "^${LIB}$" |
    xargs -r -n1 basename |
    write ${NAME}/archives
done

for LIBNAME in "${!LIBS[@]}"
do
  echo "${LIBNAME}" | write archives/${LIBS[${LIBNAME}]}
done
