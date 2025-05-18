#
# simulator.py
# PlatformIO pre: script for simulator builds
#

Import("env")

import sys
import os.path
import shutil

#
# Give the binary a distinctive name
#
env.Replace(PROGNAME="MarlinSimulator")

env.Append(
    LINKFLAGS=[
        "-static-libstdc++",
        "-static-libgcc"
    ]
)

env.Append(
    LIBS=[
        "SDL2",
        "SDL2_net",
        "GL",
        "pthread",
        "dl"
    ]
)

#
# Check for a valid GCC and available OpenGL on macOS
#

if sys.platform == 'darwin':

    emsg = ''
    fatal = 0

    gcc = shutil.which('gcc')
    if not gcc or gcc == '/usr/bin/gcc':
        if not gcc:
            emsg = "\u001b[31mNo GCC found in your configured shell PATH."
        elif gcc == '/usr/bin/gcc':
            emsg = "\u001b[31mCan't build Marlin Native on macOS using the included version of GCC (clang)."
        emsg += "\n\u001b[31mSee 'native.ini' for instructions to install GCC with MacPorts or Homebrew."
        fatal = 1
    else:
        #
        # Silence half of the ranlib warnings. (No equivalent for 'ARFLAGS')
        #
        env['RANLIBFLAGS'] += [ "-no_warning_for_no_symbols" ]

        # List of possible OpenGL header locations
        gl_paths = [
            "/opt/homebrew/include/GL/gl.h"  # Homebrew on Apple Silicon
        ]

        gl_found = None
        for path in gl_paths:
            if os.path.exists(path):
                gl_found = path
                break

        if gl_found:
            if "Xcode" in gl_found:
                env['BUILD_FLAGS'] += [ "-F" + gl_found ]
                emsg = "\u001b[33mUsing OpenGL framework headers from Xcode.app"
            else:
                env['BUILD_FLAGS'] += [ '-D__MESA__' ]
                emsg = f"\u001b[33mUsing OpenGL header from {gl_found}"
        else:
            emsg = "\u001b[31mNo OpenGL headers found. Install Xcode, MacPorts (mesa), or Homebrew (mesa)."
            fatal = 1

    # Print error message, if any
    if emsg: print(f"\n{emsg}\n")

    # Break out of the PIO build immediately
    if fatal: sys.exit(1)
