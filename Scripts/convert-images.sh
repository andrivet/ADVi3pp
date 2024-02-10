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

sketch_export_dir="$( cd "${root}/Sketch-Export" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

function copy_images() {
    echo "Copy images from ${sketch_export_dir}"

    mkdir -p "${sketch_export_dir}/DWIN_SET" "${sketch_export_dir}/Controls" "${sketch_export_dir}/Screenshots"

    find "${sketch_export_dir}" -name "*.png" -print0 | while read -r -d $'\0' file
    do
      name=$(basename "${file}")
      if [[ "${name}" == "DWIN_SET-"* ]]; then
        cp "${file}" "${sketch_export_dir}/DWIN_SET/${name#DWIN_SET-}"
      elif [[ "${name}" == "Controls-"* ]]; then
        cp "${file}" "${sketch_export_dir}/Controls/${name#Controls-}"
      elif [[ "${name}" == "Screenshots-"* ]]; then
        cp "${file}" "${sketch_export_dir}/Screenshots/${name#Screenshots-}"
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

rm "${dgus}/DWIN_SET/"*.bmp
rm "${dgus}/25_Controls/"*.bmp

copy_images
convert_images "${root}/Masters/Boot"             "${dgus}/DWIN_SET"
convert_images "${root}/Sketch-Export/DWIN_SET"   "${dgus}/DWIN_SET"
convert_images "${root}/Sketch-Export/Controls"   "${dgus}/25_Controls"
