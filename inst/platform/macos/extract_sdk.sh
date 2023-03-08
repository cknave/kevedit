#!/bin/sh
set -e -x

XCODE_VERSION="$1"
SDK_VERSION="$2"
if test -z "$XCODE_VERSION" -o -z "$SDK_VERSION"; then
    echo "USAGE: extract_sdk.sh <xcode version> <macos sdk version>"
    exit 1
fi

cd /osxcross
echo "Extracting Xcode.  This is going to take a while..."
./tools/gen_sdk_package_pbzx.sh /vendor/xcode/Xcode_${XCODE_VERSION}.xip
cp MacOSX${SDK_VERSION}.sdk.tar.bz2 /vendor/
