#!/bin/sh

# Customized settings for this program
PROJECT=kevedit        # Name of project
PARENTDIR=../../       # Location of $PROJECT directory
# Note: The directory $PROJECT must exist in $PARENTDIR

# Subdirectories of the project root ($PARENTDIR$PROJECT)
SPECSOURCEDIR=inst     # Location of $PROJECT.spec.source file
SOURCEDIR=.            # Location of source code

echo "release.sh version 0.1.0"
echo "Copyright (c) 2001  Ryan Phillips"
echo "This program may be freely distributed under the terms of the GNU GPL"
echo

if [ -z $2 ]; then
	echo "Usage: release.sh version release [--norpm]"
	echo "  version:  version string"
	echo "  release:  release number"
	echo
	echo "release.sh creates a source tgz file from a program directory."
	echo
	echo "If the --norpm parameter is not specified, binary and source RPMs"
	echo "will also be built for the project. Root priviledges may be necessary"
	echo "to build RPMs on your system."
	exit
fi

VERSION=$1
RELEASE=$2
PARAM=$3
ARCHIVE=$PROJECT-$VERSION

# Create copy of program in temporary directory
cd $PARENTDIR
cp -R $PROJECT $ARCHIVE

# Create .spec file
echo "Generating $PROJECT.spec file"
echo "Version: $VERSION" > $ARCHIVE/$PROJECT.spec
echo "Release: $RELEASE" >> $ARCHIVE/$PROJECT.spec
cat $ARCHIVE/$SPECSOURCEDIR/$PROJECT.spec.source >> $ARCHIVE/$PROJECT.spec

# Create version file
echo "Generating $PROJECT.version file"
echo "VERSIONFLAG = -D"$PROJECT"VERSION=\\\"$VERSION\\\"" > $ARCHIVE/$SOURCEDIR/$PROJECT.version
echo "VERSION = $VERSION" >> $ARCHIVE/$SOURCEDIR/$PROJECT.version

# Create optimization file
echo "Generating $PROJECT.optimize file"
echo "OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium" > $ARCHIVE/$SOURCEDIR/$PROJECT.optimize

# Create tar archive
echo "Generating" $ARCHIVE.src.tgz "archive"
tar -czf $ARCHIVE.src.tgz $ARCHIVE --exclude CVS
rm -R $ARCHIVE

if [ -z $PARAM ]; then
# Build RPM & SRPM
	echo "Building binary and source RPMS"
	rpm -ta $ARCHIVE.src.tgz
fi

