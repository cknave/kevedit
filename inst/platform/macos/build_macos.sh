#!/bin/sh
# KevEdit macOS build script
# Run in kevedit/build_macos container
set -e -x

SOURCE="$1"
VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$VERSION" ]; then
    echo "USAGE: build_macos.sh <source.zip> <kevedit version>"
    exit 1
fi

rm -rf /work/KevEdit.app
mkdir -p /work/KevEdit.app/Contents/MacOS /work/KevEdit.app/Contents/Resources
cp -a /platform/macos/Info.plist /platform/macos/PkgInfo /work/KevEdit.app/Contents

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

aclocal -I /usr/osxcross/share/aclocal
autoconf
autoheader
automake --add-missing
./configure --host=x86_64-apple-darwin14 CC=x86_64-apple-darwin14-clang CFLAGS="-O3"
make AR=x86_64-apple-darwin14-ar

# `make` builds a binary that's dynamically linked to libSDL.
# That's no good to distribute, so rebuild it with static SDL.
cd src/kevedit
rm kevedit
SDL_LIBS=$(/usr/osxcross/bin/sdl2-config --static-libs | sed -e 's/-lSDL2//')
make kevedit LIBS="$SDL_LIBS /usr/osxcross/lib/libSDL2.a"
cp -a kevedit /work/KevEdit.app/Contents/MacOS
cp -a ../../docs/kevedit.zml \
      ../../soundfx.zzm \
      ../../dosbox/kevedos.cfg \
      ../../dosbox/kevedos.iso \
      /platform/macos/kevedit.icns \
      /work/KevEdit.app/Contents/Resources/

rm -rf /work/dmg
mkdir /work/dmg

cp -a /work/KevEdit.app /work/dmg/
ln -s /Applications /work/dmg/

mkdir /work/dmg/.background
cp -a /platform/macos/background.tiff \
      /work/dmg/.background/dmg\ background.tiff

cp -a /platform/macos/dmg.DS_Store \
      /work/dmg/.DS_Store

genisoimage -V KevEdit -D -R -apple -no-pad \
    -o "/dist/kevedit-${VERSION}.dmg" \
    /work/dmg/
