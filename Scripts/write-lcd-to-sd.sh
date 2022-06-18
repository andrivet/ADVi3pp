#!/usr/bin/env zsh
: '
Create an image of the LCD panels and write it a microSD card.
Check before that the card is mounted and eject it after.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

. ./functions.sh
. ./version.sh

volumename="ADVI3PP"
volume="/Volumes/${volumename}"

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

image="${release}/ADVi3pp-LCD-${version}.img.zip"

if [[ -d ${volume} ]]; then echo "SD card is mounted, please unplug it" ; exit 1; fi

sudo -v

echo
echo "***** Create SD image and SD raw zip file..."
./create-sd-image.sh
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

if [[ ! -f "${image}" ]]; then echo "Disk image ${image} not found" ; exit 1; fi

echo
pause 'Plug-in the SD card or Ctrl-C to abort...'

echo
echo "**** Determine device name..."
sleep 2

volume="/Volumes/${volumename}"
if [[ ! -d ${volume} ]]; then echo "SD card is not mounted" ; exit 1; fi

device="$( diskutil info ${volume} | awk '/Device Identifier:/{print substr($0, 31)}' )"
if [[ -z $device ]]; then echo "Can't determine device" ; exit 1; fi

echo "Device name: ${device}"

echo
echo "***** Unmount USB device ${device}..."
sudo diskutil umount /dev/${device}
sleep 2

echo
echo "***** Burn disk image to USB..."
unzip -p "${image}" | sudo dd of=/dev/r${device} bs=1M status=progress

sleep 2
echo
echo "***** Eject USB device ${device}..."
diskutil eject "/dev/${device}"
