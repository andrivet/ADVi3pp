#!/usr/bin/env zsh
: '
Create a microSD disk image from a source directory.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi
if [[ $# -ne 4 ]] ; then echo "Invalid number of arguments" ; exit 1; fi

version=$1
label=$2
imgname=$3
compress=$4

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

dgusroot="$( cd "${scripts}/../LCD-Panel/DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${scripts}/../../../releases/v${version}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

img="${release}/${imgname}-${version}.img"

echo "Create microSD image..."
dd if=/dev/zero bs=1m count=260 of="${img}"

echo "Format as FAT32 8 sectors per cluster..."
mkfs.fat -F 32 -n "${label}" -s 8 -v "${img}"

echo "Mount the SD image..."
mount=$( sudo hdiutil attach -readwrite -imagekey diskimage-class=CRawDiskImage "${img}" | awk '{print $2}' )
if [[ "${mount}" == "" ]]; then echo "Mounting failed" ; exit 1; fi

echo "Copy files..."
mkdir -p "${mount}/DWIN_SET/"
cp -R -v "${dgusroot}/DWIN_SET/"* "${mount}/DWIN_SET/"

echo "Clean the files..."
find "${mount}" -name '.DS_Store' -delete
find "${mount}" -name '._*' -type f -delete

echo "Detach the SD image..."
sudo hdiutil detach "${mount}"

echo "Compress the SD image..."
zip -j "${img}.zip" "${img}"
rm "${img}"

if [[ "${compress}" = "1" ]]; then
  echo "Compress the images in ${dgusroot} into a zip file..."
  pushd "${dgusroot}" >/dev/null || exit 1
  zip -r -x@"${scripts}/excludes.txt" "${release}/${imgname}-${version}.zip" -- *
  popd >/dev/null || exit 1
fi
