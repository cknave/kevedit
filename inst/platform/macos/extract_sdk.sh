#!/bin/sh
set -e -x

XCODE_VERSION="$1"
if [ -z "$XCODE_VERSION" ]; then
    echo "USAGE: extract_sdk.sh <xcode version>"
    exit 1
fi

xar -xf "/vendor/xcode/Xcode${XCODE_VERSION}.xip" -C /tmp
cd /tmp
echo "Extracting Xcode.  This is going to take a while..."
pbzx -n Content | cpio -i

cd /vendor
XCODEDIR=/tmp/Xcode.app gen_sdk_package.sh
