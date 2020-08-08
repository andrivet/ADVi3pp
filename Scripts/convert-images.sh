#!/usr/bin/env zsh
: '
Clean the old bitmaps from the DGUS directories 
and convert the PNG images into proper bitmaps for the LCD Panel (BMP3).
Must be called each time the images are modified.

This script requires Sketch application to work (macOS). If you do not have it, there are
third-parties open-source projects able to extract resources from a sketch file. Google is your friend.
'

sketchtool="/Applications/Sketch.app/Contents/Resources/sketchtool/bin/sketchtool"
[[ -f ${sketchtool} ]] || { echo "Sorry, this script requires Sketch app (macOS) in order to work."; exit 1; }

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

root="$( cd "${scripts}/../LCD-Panel" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgus="$( cd "${root}/DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

sketch_export_dir="$( cd "${root}/Sketch-Export" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

masters="$( cd "${root}/Masters" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

sketch="${masters}/ADVi3++5 LCD-Panel.sketch"

function export_sketches() {
    echo "Export Sketch ${sketch} into ${sketch_export_dir}"
    "${sketchtool}" export artboards --output="${sketch_export_dir}" --overwriting=YES "${sketch}"
    # Fix a bug with sketchtool
    mv "${sketch_export_dir}/"*.png "${sketch_export_dir}/Screenshots/"
}

function clean_files() {

    echo "Clean $2 files in $1..."
    rm "$1/"*."$2"

}

function convert_images() {

    mkdir -p "$2"

    echo "Convert images from $1 to 24 bit BMP and copy them into $2..."
    for f in "$1/"*.png ; do
        filename=$(basename "$f")
        name="${filename%.*}"
        echo Convert "${name}"
        convert "$f" "BMP3:$2/${name}.bmp"
        ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
    done

}

clean_files "${sketch_export_dir}/DWIN_SET" "png"
clean_files "${sketch_export_dir}/Controls" "png"
clean_files "${sketch_export_dir}/Screenshots" "png"
clean_files "${dgus}/DWIN_SET" "bmp"
clean_files "${dgus}/25_Controls" "bmp"

export_sketches
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

convert_images "${root}/Masters/Boot"             "${dgus}/DWIN_SET"
convert_images "${root}/Sketch-Export/DWIN_SET"   "${dgus}/DWIN_SET"
convert_images "${root}/Sketch-Export/Controls"   "${dgus}/25_Controls"
