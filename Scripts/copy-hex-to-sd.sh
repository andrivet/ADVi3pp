#!/usr/bin/env zsh
: '
Copy .hex files to a microSD card. 
Check before that the card is mounted and eject it after.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

if [[ $# == 0 ]]; then echo "Please provide a version number such as 1.0.0" ; exit 1; fi
version=$1

volumename="ADVI3PP"
volume="/Volumes/${volumename}"

while [[ ! -d ${volume} ]]; do
    echo "SD card is not mounted"
    sleep 2
done

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo "Copy .hex files..."
cp "${release}/"*.hex "${volume}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo "Remove macOS Spotlight files..."
sleep 2
sudo mdutil -d "${volume}"
sleep 2
sudo mdutil -X "${volume}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

sleep 2
diskutil eject "${volume}"
