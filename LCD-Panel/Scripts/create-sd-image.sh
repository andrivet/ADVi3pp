#!/usr/bin/env zsh
: '
Create a microSD disk image for the LCD Panel.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

if [[ $# -eq 0 ]] ; then echo "Please provide a version number such as 1.0.0" ; exit 1; fi

version=$1

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

src="$( cd "${scripts}/../DGUS-root" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../../Releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

img=${release}/ADVi3pp-LCD-${version}.img

echo "Create microSD image..."
dd if=/dev/zero bs=1m count=512 of=${img}

echo "Format as FAT32 8 sectors per cluster..."
mkfs.fat -F 32 -n ADVI3PP -s 8 -v ${img}

echo "Mount the SD image..."
mount=$( sudo hdiutil attach -readwrite -imagekey diskimage-class=CRawDiskImage ${img} | awk '{print $2}' )

echo "Copy files..."
mkdir -p "${mount}/DWIN_SET/"
cp -R -v "${src}/DWIN_SET/"* "${mount}/DWIN_SET/"

echo "Clean the files..."
find ${mount} -name '.DS_Store' -delete
find ${mount} -name '._*' -type f -delete

echo "Detach the SD image..."
sudo hdiutil detach ${mount}

echo "Compress the SD image..."
zip -j ${img}.zip ${img}
rm ${img}

echo "Compress the images in ${src} into a zip file..."
pushd "${src}" >/dev/null
zip -r -x@${scripts}/excludes.txt "${release}/ADVi3pp-LCD-${version}.zip" *
popd >/dev/null

