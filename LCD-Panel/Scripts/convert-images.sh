#!/usr/bin/env zsh
: '
Clean the old bitmaps from the DGUS directories 
and convert the PNG images into proper bitmaps for the LCD Panel (BMP3).
Must be called each time the images are modified.
'

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

parent="$( cd "${scripts}/.." && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgus="$( cd "${scripts}/../DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

sketchtool="/Applications/Sketch.app/Contents/Resources/sketchtool/bin/sketchtool"
sketch_export_dir="$( cd "${scripts}/../Sketch-Export" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
masters="$( cd "${scripts}/../Masters" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
sketch="${masters}/3DLabs LCD-Panel.sketch"

function export_sketches() {
    echo Export Sketches into ${sketch_export_dir}
    "${sketchtool}" export artboards --output=${sketch_export_dir} --overwriting=YES ${sketch}
    # Fix a bug with sketchtool
    mv "${sketch_export_dir}/"*.png "${sketch_export_dir}/Screenshots/"
}

function clean_images() {

    echo Clean bmp files in $1...
    rm $1/*.bmp

}

function convert_images() {

    mkdir -p $2

    echo Convert images from $1 to 24 bit BMP and copy them into $2...
    for f in "$1/"*.png ; do
        filename=$(basename "$f")
        name="${filename%.*}"
        echo Convert "${name}"
        convert "$f" "BMP3:$2/${name}.bmp"
        ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
    done

}

clean_images "${dgus}/DWIN_SET"
clean_images "${dgus}/25_Controls"

export_sketches

convert_images "${parent}/Masters/Boot"             "${dgus}/DWIN_SET"
convert_images "${parent}/Sketch-Export/DWIN_SET"   "${dgus}/DWIN_SET"
convert_images "${parent}/Sketch-Export/Controls"   "${dgus}/25_Controls"
