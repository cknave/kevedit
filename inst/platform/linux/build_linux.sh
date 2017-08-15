#!/bin/sh
set -e -x

SOURCE="$1"
if [ -z "$SOURCE" ]; then
    echo "USAGE: build_linux.sh <source.zip>"
    exit 1
fi

rm -rf /work/appdir
mkdir /work/appdir
cp -a /platform/linux/KevEdit.AppDir /work/appdir
cp /vendor/AppRun-x86_64 /work/appdir/KevEdit.AppDir/AppRun

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

# /usr/local not included for some reason, skip bootstrap and set up manually
aclocal -I /usr/local/share/aclocal
autoconf
autoheader
automake --add-missing
./configure --prefix=/work/appdir/KevEdit.AppDir/usr CFLAGS='-O3'
make
make install
