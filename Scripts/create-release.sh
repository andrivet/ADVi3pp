#!/usr/bin/env zsh
: '
Create a new release.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

function pause(){
   read -r -s -k "?$*"$'\n'
}

version="5.1.1"

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

mkdir -p "${scripts}/../../../releases/v${version}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

advi3pp="$( cd "${scripts}/../" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo "======================================"
echo "Create ADVi3++ version ${version}"
echo "======================================"
echo
echo "BE SURE YOU HAVE CHANGED:"
echo "- CHANGELOG.md"
echo "- advbumpversion --allow-dirty --no-commit --no-tag release"
echo "- User Manual if necessary"
echo
pause 'Press any key to continue or Ctrl-C to abort...'

sudo -v

rm -rf "${release:?}/*"

echo
echo "***** Create SD image and SD raw zip file..."
./create-sd-image.sh "${version}"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Generate other microSD images..."
./create-sd-calibration-image.sh "${version}"
./create-sd-reset-image.sh "${version}"

echo
echo "***** Compile Mainboard firmwares..."
echo
platformio -f -c clion run -d "${advi3pp}" -e advi3pp_51 -e advi3pp_51_bltouch -e advi3pp_52c -e advi3pp_52c_bltouch -e advi3pp_54
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Copy Mainboard firmwares..."
echo
cp "${advi3pp}/.pio/build/advi3pp_51/firmware.hex" "${release}/ADVi3pp-Mainboard-51-${version}.hex" && \
cp "${advi3pp}/.pio/build/advi3pp_51_bltouch/firmware.hex" "${release}/ADVi3pp-Mainboard-51-BLTouch-${version}.hex"
cp "${advi3pp}/.pio/build/advi3pp_52c/firmware.hex" "${release}/ADVi3pp-Mainboard-52c-${version}.hex"
cp "${advi3pp}/.pio/build/advi3pp_52c_bltouch/firmware.hex" "${release}/ADVi3pp-Mainboard-52c-BLTouch-${version}.hex"
cp "${advi3pp}/.pio/build/advi3pp_54/firmware.hex" "${release}/ADVi3pp-Mainboard-54-${version}.hex"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

echo
echo "***** Copy Arduino Core..."
echo
pushd "${advi3pp}" >/dev/null || exit
zip -r -x@"${scripts}"/excludes-core.txt "${release}/ArduinoCore-${version}.zip" ArduinoCore
ret=$?; if [[ $ret != 0 ]]; then popd && exit $ret; fi
popd >/dev/null || exit

echo
echo "**** ADVi3++ ${version} is ready in ${release}"
echo
echo "Add a tag in Git and push the release"
echo
