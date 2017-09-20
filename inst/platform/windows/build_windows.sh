#!/bin/sh
set -e -x

SOURCE="$1"
SDL_VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$SDL_VERSION" ]; then
    echo "USAGE: build_windows.sh <source.zip> <sdl version>"
    exit 1
fi

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

aclocal -I /usr/x86_64-w64-mingw32/share/aclocal
autoconf
autoheader
automake --add-missing
./configure --host=x86_64-w64-mingw32 CFLAGS="-O3"
make

# Add txt extension to docs
# TODO: fix dos newlines
for fn in AUTHORS ChangeLog COPYING; do
    cp /work/kevedit/$fn /work/$fn.txt
done

# Extract SDL runtime
rm -rf /work/sdl
mkdir /work/sdl
cd /work/sdl
unzip -j /vendor/SDL2-${SDL_VERSION}-win32-x64.zip

HOME=/work wine /innosetup/app/ISCC.exe - </work/kevedit/inst/platform/windows/kevedit.iss
