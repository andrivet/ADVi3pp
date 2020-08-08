#!/usr/bin/env zsh
: '
Copy required files to a microSD card. 
Check before that the card is mounted and eject it after.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

volumename="ADVI3PP"
volume="/Volumes/${volumename}"

if [[ ! -d ${volume} ]]; then echo "SD card is not mounted" ; exit 1; fi

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgus="$( cd "${scripts}/../LCD-Panel/DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo "Clean the microSD..."
sudo rm -Rf "${volume}/DWIN_SET/"*
echo "Copy files..."
cp -R "${dgus}/DWIN_SET/" "${volume}/DWIN_SET/"
sleep 2
diskutil eject "${volume}"
