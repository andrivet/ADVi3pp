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

funcion clean_images() {

    echo Clean bmp files in $1...
    [[ -e $1/*.bmp ]] || rm $1/*.bmp
    ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

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

convert_images "${parent}/Masters/Boot"             "${dgus}/DWIN_SET"
convert_images "${parent}/Sketch-Export/DWIN_SET"   "${dgus}/DWIN_SET"
convert_images "${parent}/Sketch-Export/Controls"   "${dgus}/25_Controls"
