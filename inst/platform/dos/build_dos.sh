#!/bin/sh
# KevEdit DOS build script
# Run in kevedit/build_dos container
set -e -x

SOURCE="$1"
VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$VERSION" ]; then
    echo "USAGE: build_dos.sh <source.zip> <kevedit version>"
    exit 1
fi

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

./bootstrap.sh
./configure --host=i586-pc-msdosdjgpp --without-dosbox --without-sdl CFLAGS='-O3'
make
i586-pc-msdosdjgpp-strip src/kevedit/kevedit.exe

rm -rf /work/kevedit_zip
mkdir /work/kevedit_zip
cp src/kevedit/kevedit.exe \
   docs/kevedit.zml \
   soundfx.zzm \
   inst/platform/dos/cwsdpmi.exe \
   /work/kevedit_zip/

# Add .txt and fix newlines
for fn in AUTHORS ChangeLog COPYING; do
    sed -e 's/$/\r/' <$fn >/work/kevedit_zip/$fn.txt
done
sed -e 's/$/\r/' <README.md >/work/kevedit_zip/README.md

dist_zip=/dist/kevedit-${VERSION}-dos.zip
rm -f "$dist_zip"
cd /work/kevedit_zip
zip -9 "$dist_zip" *
