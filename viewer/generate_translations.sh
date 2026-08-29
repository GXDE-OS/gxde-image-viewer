#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

if [ -z "${LRELEASE}" ]; then
    LRELEASE=$(command -v lrelease || true)
fi
if [ -z "${LRELEASE}" ] && [ -x /usr/lib/qt6/bin/lrelease ]; then
    LRELEASE=/usr/lib/qt6/bin/lrelease
fi
if [ -z "${LRELEASE}" ]; then
    echo "lrelease not found; please install qt6-l10n-tools" >&2
    exit 1
fi

ts_list=(`ls translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    "${LRELEASE}" "${ts}"
done
