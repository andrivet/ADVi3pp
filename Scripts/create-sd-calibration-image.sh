#!/usr/bin/env zsh
: '
Create a microSD disk image for the calibration of the LCD Panel.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

. ./version.sh
./create-sd-image-from-dir.sh "Calibration" "ADVCALIB" "ADVi3pp-CALIB-LCD" 0
