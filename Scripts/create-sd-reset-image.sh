#!/usr/bin/env zsh
: '
Create a microSD disk image for the calibration of the LCD Panel.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi
if [[ $# -ne 1 ]] ; then echo "Please provide a version number such as 1.0.0" ; exit 1; fi

version=$1

./create-sd-image-from-dir.sh "${version}" "Calibration" "ADVCALIB" "ADVi3pp-CALIB-LCD" 0
