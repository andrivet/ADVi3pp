#!/usr/bin/env zsh
: '
Create a microSD disk image for the LCD Panel.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi
if [[ $# -ne 1 ]] ; then echo "Please provide a version number such as 1.0.0" ; exit 1; fi

version=$1

echo
echo "***** Convert images..."
echo
./convert-boot-images.sh
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
./convert-images.sh
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

./create-sd-image-from-dir.sh "${version}" "DGUS-root" "ADVI3PP" "ADVi3pp-LCD" 1

