#!/bin/sh

# Customized settings for this program
PROJECT=kevedit        # Name of project
PARENTDIR=../../       # Location of $PROJECT directory
# Note: The directory $PROJECT must exist in $PARENTDIR

# Subdirectories of the project root ($PARENTDIR$PROJECT)
SPECSOURCEDIR=inst     # Location of $PROJECT.spec.source file
SOURCEDIR=.            # Location of source code

echo "mkrpmtar version 0.0.3"
echo "Copyright (c) 2001  Ryan Phillips"
echo "This program may be freely distributed under the terms of the GNU GPL"
echo

if [ -z $2 ]; then
	echo "Usage: mkrpmtar.sh version release"
	echo "  version:  version string"
	echo "  release:  release number"
	echo
	echo "mkrpmtar creates a tar file from a program directory and uses it"
	echo "to build source and binary rpms for that program."
	exit
fi

VERSION=$1
RELEASE=$2
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

# Build RPM & SRPM
echo "Building binary and source RPMS"
rpm -ta $ARCHIVE.src.tgz
