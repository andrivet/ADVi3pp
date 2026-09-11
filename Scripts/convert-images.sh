#!/usr/bin/env zsh
: '
Convert the PNG images into proper bitmaps for the LCD Panel (BMP3).
Must be called each time the images are modified.
'

unsetopt nomatch

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

root="$( cd "${scripts}/../LCD-Panel" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgus="$( cd "${root}/DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${root}/DGUS-root/25_Controls"
mkdir -p "${root}/DGUS-png"
mkdir -p "${root}/Export"

png="$( cd "${root}/DGUS-png" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

export="$( cd "${root}/Export" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

quiet=false
if [[ "$1" == "--quiet" ]]; then
    quiet=true
fi

function clean_export() {
    echo "Clean images from ${export}"
    rm -r "${export:?}"/*
}

function copy_images() {
    echo "Copy images from ${export} to ${png}"

    mkdir -p "${png}/DWIN_SET" "${png}/Controls" "${png}/Screenshots" "${png}/Images"

    find "${export}" -name "*.png" -print0 | while read -r -d $'\0' file
    do
      name=$(basename "${file}")
      if [[ "${name}" == "DWIN_SET-"* ]]; then
        magick "${file}" -format png -background black -flatten "${png}/DWIN_SET/${name#DWIN_SET-}"
      elif [[ "${name}" == "Widget-"* ]]; then
        magick "${file}" -format png -background black -flatten "${png}/Controls/${name#Widget-}"
      elif [[ "${name}" == "Screenshots-"* ]]; then
        magick "${file}" -format png -background black -flatten "${png}/Screenshots/${name#Screenshots-}"
      elif [[ "${name}" == "Image-"* ]]; then
        cp "${file}" "${png}/Images/${name#Image-}"
      else
        echo WARNING: Unknown file "${file}"
      fi
    done
}

function convert_images() {
    echo "Convert images from $1 to 24 bit BMP and copy them into $2..."
    for f in "$1/"*.png ; do
        filename=$(basename "$f")
        name="${filename%.*}"
        magick "$f" -type truecolor "BMP3:$2/${name}.bmp"
        ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
    done
}

if ! $quiet ; then
  if read -q "answer?Clean Export? "; then
    print "\n"
    clean_export
    print "\nPlease, export the images."
  else
    printf "\nFiles not cleaned\n"
  fi
  if ! read -q "answer?Continue? "; then
    printf "\nAbort."
    exit 1
  fi
  printf "\n"
fi

rm -rf "${png}/DWIN_SET" "${png}/Controls" "${png}/Screenshots"
rm -f "${dgus}/DWIN_SET/"*.bmp
rm -f "${dgus}/25_Controls/"*.bmp

copy_images
convert_images "${png}/Boot"            "${dgus}/DWIN_SET"
convert_images "${png}/DWIN_SET"        "${dgus}/DWIN_SET"
convert_images "${png}/Controls"        "${dgus}/25_Controls"
