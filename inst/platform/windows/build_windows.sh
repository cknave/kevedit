#!/bin/sh
# KevEdit windows build script
# Run in kevedit/build_windows container
set -e -x

SOURCE="$1"
KEVEDIT_VERSION="$2"
SDL_VERSION="$3"
if [ -z "$SOURCE" ] || [ -z "$KEVEDIT_VERSION" ] || [ -z "$SDL_VERSION" ]; then
    echo "USAGE: build_windows.sh <source.zip> <kevedit version> <sdl version>"
    exit 1
fi
export KEVEDIT_VERSION

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

./bootstrap.sh
./configure --host=x86_64-w64-mingw32 --target=x86_64-w64-mingw32 CFLAGS="-O3"
make

# Add .txt and fix newlines
for fn in AUTHORS ChangeLog COPYING; do
    cp /work/kevedit/$fn /work/$fn.txt
    sed -e 's/$/\r/' </work/kevedit/$fn >/work/$fn.txt
done
# Fix markdown newlines
for fn in README.md legal.md; do
    sed -i -e 's/$/\r/' /work/kevedit/$fn
done

# Extract SDL runtime
rm -rf /work/sdl
mkdir /work/sdl
cd /work/sdl
unzip -j /vendor/SDL2-${SDL_VERSION}-win32-x64.zip

HOME=/work wine /innosetup/app/ISCC.exe - </work/kevedit/inst/platform/windows/kevedit.iss
mv /dist/mysetup.exe /dist/kevedit-${KEVEDIT_VERSION}-setup.exe
