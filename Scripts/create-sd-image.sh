#!/usr/bin/env zsh
: '
Create a microSD disk image for the LCD Panel.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

echo
echo "***** Convert images..."
echo
./generate-boot-images.sh
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi
./convert-images.sh --quiet
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

./create-sd-image-from-dir.sh "DGUS-root" "ADVI3PP" "ADVi3pp-LCD" 1

