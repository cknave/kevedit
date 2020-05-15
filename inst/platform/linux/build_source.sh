#!/bin/sh
# KevEdit source distribution build script
# Run in kevedit/build_source container
set -e -x

SOURCE="$1"
VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$VERSION" ]; then
    echo "USAGE: build_linux.sh <source.zip> <version>"
    exit 1
fi

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

./bootstrap.sh --configure
make -C build distcheck
mv build/kevedit-*.tar.gz "/dist/kevedit-$VERSION.tar.gz"
