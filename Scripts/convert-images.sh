#!/usr/bin/env zsh
: '
Convert the PNG images into proper bitmaps for the LCD Panel (BMP3).
Must be called each time the images are modified.
'

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

root="$( cd "${scripts}/../LCD-Panel" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgus="$( cd "${root}/DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

png="$( cd "${root}/DGUS-png" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

export="$( cd "${root}/Export" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

function clean_export() {
    echo "Clean images from ${export}"
    rm -r "${export:?}"/*
}

function copy_images() {
    echo "Copy images from ${export} to ${png}"

    mkdir -p "${png}/DWIN_SET" "${png}/Controls" "${png}/Screenshots"

    find "${export}" -name "*.png" -print0 | while read -r -d $'\0' file
    do
      name=$(basename "${file}")
      if [[ "${name}" == "DWIN_SET-"* ]]; then
        cp "${file}" "${png}/DWIN_SET/${name#DWIN_SET-}"
      elif [[ "${name}" == "Controls-"* ]]; then
        cp "${file}" "${png}/Controls/${name#Controls-}"
      elif [[ "${name}" == "Screenshots-"* ]]; then
        cp "${file}" "${png}/Screenshots/${name#Screenshots-}"
      else
        echo WARNING: Unknown file "${file}"
      fi
    done
}

function convert_images() {
    mkdir -p "$2"

    echo "Convert images from $1 to 24 bit BMP and copy them into $2..."
    for f in "$1/"*.png ; do
        filename=$(basename "$f")
        name="${filename%.*}"
        convert "$f" -type truecolor "BMP3:$2/${name}.bmp"
        ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
    done
}

if read -q "answer?Clean Export? "; then
  printf "\n"
  clean_export
fi

print "\nPlease, export the images."
if read -q "answer?Continue? "; then
  printf "\n"
  copy_images
  convert_images "${png}/Boot"            "${dgus}/DWIN_SET"
  convert_images "${png}/DWIN_SET"        "${dgus}/DWIN_SET"
  convert_images "${png}/Controls"        "${dgus}/25_Controls"
else
  printf "\nAbort."
fi
