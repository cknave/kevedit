#!/bin/sh

# Customized settings for this program
PROJECT=kevedit        # Name of project
MAKEFILE=Makefile      # Default makefile
PARENTDIR=../../       # Location of $PROJECT directory
# Note: The directory $PROJECT must exist in $PARENTDIR

# Subdirectories of the project root ($PARENTDIR$PROJECT)
SPECSOURCEDIR=inst     # Location of $PROJECT.spec.source file
SOURCEDIR=.            # Location of source code

THIS="`basename $0`"
HERE="`dirname $0`"
echo "$THIS version 0.2.1"
echo "Copyright (c) 2001  Ryan Phillips <bitman@users.sf.net>"
echo "This program may be freely distributed under the terms of the GNU GPL"
echo

# At least two parameters required
if [ $# -lt 2 ]; then
	echo "Usage: $THIS version release [makefile] [--nobin] [--norpm] [--nosrc]"
	echo "  version:  version string"
	echo "  release:  release number"
	echo "  makefile: makefile to use in binary release"
	echo "  --nobin:  do not generate binary release"
	echo "  --norpm:  do not generate binary and source rpms"
	echo "  --nosrc:  do not generate binary release"
	echo
	echo "$THIS creates a source tarball from a program directory. A "
	echo "binary release, binary rpm and source rpm will also be generated,"
	echo "unless otherwise specified."
	echo
	echo "Note: root priviledges may be necessary to build rpms on your system."
	# Bad args
	exit 65
fi

VERSION=$1; shift
RELEASE=$1; shift
ARCHIVE=$PROJECT-$VERSION

# Process arguments
for arg in "$@"; do
	case $arg in
		--nosrc) NOSRC="true"; echo "$THIS: Not building source tarball";;
		--nobin) NOBIN="true"; echo "$THIS: Not building binary tarball";;
		--norpm) NORPM="true"; echo "$THIS: Not building rpm/srpm";;
		--*) echo "Ignoring unknown option: $arg";;
		*) MAKEFILE=$arg; echo "Using makefile: $arg";;
	esac
done

# Create copy of program in temporary directory
cd $HERE/$PARENTDIR
cp -R $PROJECT $ARCHIVE

# Remember the release root
ROOT=`pwd`
echo "$THIS: Storing release tarballs in: $ROOT"

# Create .spec file
echo
echo "$THIS: Generating $PROJECT.spec file"
echo "Version: $VERSION" > $ARCHIVE/$PROJECT.spec
echo "Release: $RELEASE" >> $ARCHIVE/$PROJECT.spec
cat $ARCHIVE/$SPECSOURCEDIR/$PROJECT.spec.source >> $ARCHIVE/$PROJECT.spec

# Create version file
echo "$THIS: Generating $PROJECT.version file"
echo "VERSIONFLAG = -D"$PROJECT"VERSION=\\\"$VERSION\\\"" > $ARCHIVE/$SOURCEDIR/$PROJECT.version
echo "VERSION = $VERSION" >> $ARCHIVE/$SOURCEDIR/$PROJECT.version

# Create optimization file
echo "$THIS: Generating $PROJECT.optimize file"
echo "OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium" > $ARCHIVE/$SOURCEDIR/$PROJECT.optimize

# Clean source
echo
echo "$THIS: Cleaning the source"
cd $ARCHIVE
make clean
cd $ROOT

if [ -z $NOSRC ]; then
	# Create tarball source release
	echo
	echo "$THIS: Generating $ARCHIVE.src.tgz source release tarball"
	tar -czf $ARCHIVE.src.tgz $ARCHIVE --exclude CVS
fi

if [ -z $NOBIN ]; then
	# Build binary tarball

	# Build the binaries
	echo
	echo "$THIS: Building binary release"
	cd $ARCHIVE
	make -f $MAKEFILE root=

	# Make an $ARCHIVE-release subdirectory to install the binaries to
	mkdir $ARCHIVE-release
	make -f $MAKEFILE root=$ARCHIVE-release install

	# Create the binary tar archive
	echo "$THIS: Generating $ARCHIVE.tgz binary release tarball"
	cd $ARCHIVE-release
	tar -czf $ROOT/$ARCHIVE.tgz *

	# Return to the release root
	cd $ROOT

	# End build binary tarball
fi

# Remove the build directory
rm -R $ARCHIVE

if [ -z $NORPM ]; then
	# Build RPM & SRPM
	echo
	echo "$THIS: Building binary and source RPMS"
	rpm -ta $ARCHIVE.src.tgz
	echo
	echo "$THIS: RPMs and SRPMs can be found wherever rpm put them (see above)"
else
	echo
fi

echo "$THIS: Release tarballs can be found in $ROOT"
