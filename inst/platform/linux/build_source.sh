#!/bin/sh
# KevEdit source distribution build script
# Run in kevedit/build_source container
set -e -x

SOURCE="$1"
KEVEDIT_VERSION="$2"
if [ -z "$SOURCE" ] || [ -z "$KEVEDIT_VERSION" ]; then
    echo "USAGE: build_linux.sh <source.zip> <version>"
    exit 1
fi
export KEVEDIT_VERSION

rm -rf /work/kevedit
mkdir /work/kevedit
cd /work/kevedit

unzip /vendor/$SOURCE

./bootstrap.sh --configure
make -C build distcheck
mv build/kevedit-*.tar.gz /dist
