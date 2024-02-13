#!/usr/bin/env zsh

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

root="$( cd "${scripts}/../LCD-Panel" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

masters="$( cd "${root}/Masters" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

png="$( cd "${root}/DGUS-png" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${png}/Boot/"

for i in {0..20}
do
  # shellcheck disable=SC2079
  convert "${masters}"/Boot.png -alpha set -background opaque -channel A -evaluate multiply $((i*1.0/20)) +channel -flatten "PNG24:${png}/Boot/$(printf "%03d" $((i+200)))_boot.png"
done
