#!/bin/sh
# KevEdit macOS build script
# Run in kevedit/build_macos container
set -e -x

SOURCE="$1"
KEVEDIT_VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$KEVEDIT_VERSION" ]; then
    echo "USAGE: build_macos.sh <source.zip> <kevedit version>"
    exit 1
fi
export KEVEDIT_VERSION

export MACOSX_DEPLOYMENT_TARGET=10.7

rm -rf /work/KevEdit.app
mkdir -p \
    /work/KevEdit.app/Contents/MacOS \
    /work/KevEdit.app/Contents/Resources \
    /work/KevEdit.app/Contents/Frameworks
cp -a /platform/macos/PkgInfo /work/KevEdit.app/Contents

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

./bootstrap.sh
# Build binaries for both x86_64 and arm64
for arch in x86_64 arm64; do
    test -f Makefile && make clean
    ./configure \
        --host=$arch-apple-darwin20.4 \
        --target=$arch-apple-darwin20.4 \
        --with-sdl-framework \
        CFLAGS="-O3"
    make AR=llvm-ar
    mv src/kevedit/kevedit /work/kevedit.bin.$arch
done

# Combine them into a universal binary
lipo -create \
    -output /work/KevEdit.app/Contents/MacOS/kevedit \
    /work/kevedit.bin.*

cp -a docs/kevedit.zml \
      soundfx.zzm \
      dosbox/kevedos.cfg \
      dosbox/kevedos.iso \
      /platform/macos/kevedit.icns \
      /work/KevEdit.app/Contents/Resources/
cp -a platform/macos/Info.plist work/KevEdit.app/Contents/

`osxcross-conf`
cp -a $OSXCROSS_SDK/Library/Frameworks/SDL2.framework /work/KevEdit.app/Contents/Frameworks
rm /work/KevEdit.app/Contents/Frameworks/SDL2.framework/Headers
rm -rf /work/KevEdit.app/Contents/Frameworks/SDL2.framework/Versions/Current/Headers

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
    -o "/dist/kevedit-${KEVEDIT_VERSION}.dmg" \
    /work/dmg/
