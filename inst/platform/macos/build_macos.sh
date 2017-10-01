#!/bin/sh
set -e -x

SOURCE="$1"
if [ -z "$SOURCE" ]; then
    echo "USAGE: build_macos.sh <source.zip>"
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
cp -a ../../docs/kevedit.zml ../../soundfx.zzm /work/KevEdit.app/Contents/Resources/

rm -rf /dist/KevEdit.app
cp -a /work/KevEdit.app /dist
