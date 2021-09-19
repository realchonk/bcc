#!/bin/sh

ver="0.16"

commit="$(git describe --always 2>/dev/null)"

if [ -n "${commit}" ]; then
   echo "${ver}-${commit}"
else
   echo "${ver}"
fi
