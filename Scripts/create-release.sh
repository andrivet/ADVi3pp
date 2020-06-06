#!/usr/bin/env zsh
: '
Create a new release.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

function pause(){
   read -s -k "?$*"$'\n'
}

if [[ $# == 0 ]]; then echo "Please provide a version number such as 1.0.0" ; exit 1; fi
version=$1

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${scripts}/../../../releases/v${version}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

advi3pp="$( cd "${scripts}/../" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo "=============================="
echo "Create ADVi3++ $1"
echo "=============================="
echo
echo "BE SURE YOU HAVE CHANGED:"
echo "- CHANGELOG.md"
echo "- advbumpversion: advbumpversion --allow-dirty release --list"
echo "- User Manual if necessary"
echo
pause 'Press any key to continue or Ctrl-C to abort...'

rm -rf "${release}/*"

echo
echo "***** Create SD image and SD raw zip file..."
echo
./create-sd-image.sh $1
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Compile Mainboard firmwares..."
echo
platformio -f -c clion run -d "${advi3pp}" -e i3plus -e i3plus-bltouch -e i3plus-bltouch3 -e i3plus-mark2 -e i3plus-he180021 -e i3plus-he180021-bltouch -e i3plus-he180021-bltouch3
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Copy Mainboard firmwares..."
echo
cp "${advi3pp}/.pioenvs/i3plus/firmware.hex" "${release}/ADVi3pp-Mainboard-${version}.hex" && \
cp "${advi3pp}/.pioenvs/i3plus-bltouch/firmware.hex" "${release}/ADVi3pp-BLTouch-Mainboard-${version}.hex"
cp "${advi3pp}/.pioenvs/i3plus-bltouch3/firmware.hex" "${release}/ADVi3pp-BLTouch3-Mainboard-${version}.hex"
cp "${advi3pp}/.pioenvs/i3plus-mark2/firmware.hex" "${release}/ADVi3pp-Mark2-Mainboard-${version}.hex"
cp "${advi3pp}/.pioenvs/i3plus-he180021/firmware.hex" "${release}/ADVi3pp-HE180021-Mainboard-${version}.hex"
cp "${advi3pp}/.pioenvs/i3plus-he180021-bltouch/firmware.hex" "${release}/ADVi3pp-HE180021-BLTouch-Mainboard-${version}.hex"
cp "${advi3pp}/.pioenvs/i3plus-he180021-bltouch3/firmware.hex" "${release}/ADVi3pp-HE180021-BLTouch3-Mainboard-${version}.hex"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Copy Arduino Core..."
echo
pushd "${advi3pp}" >/dev/null
zip -r -x@${scripts}/excludes-core.txt "${release}/ArduinoCore-${version}.zip" ArduinoCore
ret=$?; if [[ $ret != 0 ]]; then popd && exit $ret; fi
popd >/dev/null

echo
echo "**** ADVi3++ ${version} is ready in ${release}"
echo
echo "Add a tag in Git and push the release"
echo
