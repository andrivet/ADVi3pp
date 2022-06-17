#!/usr/bin/env zsh
: '
Upload a firmware.
'

if [[ "$OSTYPE" != "darwin"* ]]; then echo "Work only on macOS, sorry" ; exit 1; fi

. ./version.sh

imgname=ADVi3pp-Mainboard-51-BLTouch

scripts="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

release="$( cd "${scripts}/../../../releases/v${version}" && pwd )"
ret=$?; if [[ $ret != 0 ]]; then exit $ret; fi

img="${release}/${imgname}-${version}.hex"

avrdude -C${HOME}/.platformio/packages/tool-avrdude/avrdude.conf -pm2560 -Pusb -b115200 -catmelice -U flash:w:"${img}":i
