#!/usr/bin/env zsh
unsetopt nomatch

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

root="$( cd "${scripts}/../LCD-Panel" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

masters="$( cd "${root}/Masters" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${root}/DGUS-png"
png="$( cd "${root}/DGUS-png" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${png}/Boot/"
rm -f "${png}/Boot/"*.png

nb_images=10
for ((i=1; i < nb_images; i++))
do
  # shellcheck disable=SC2079
  magick "${masters}"/Boot.png -alpha set -background opaque -channel A -evaluate multiply $((i*1.0/nb_images)) +channel -flatten "PNG24:${png}/Boot/$(printf "%03d" $((i-1+240)))_boot.png"
done

magick "${masters}"/Boot.png "${masters}"/Connecting.png -layers flatten ${png}/Boot/249_boot.png
