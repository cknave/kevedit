#!/bin/sh

BUILDDIR=build/

for arg in "$@"; do
	case $arg in
		--noreconf) NORECONF=yes;;
		--clear) CLEAR=yes NORECONF=yes;;
		--rebuild) CLEAR=yes;;
		--configure) CONFIGURE=yes;;
		--make) MAKE=yes;;
		--install) INSTALL=yes;;
		--all) CONFIGURE=yes; MAKE=yes; INSTALL=yes;;
	esac
done

if [ -z $CLEAR ]; then echo; else
	echo "Deleting files created by automake and autoconf"
	rm -f aclocal.m4 config.* configure depcomp install install-sh missing mkinstalldirs
	rm -f *.lnk
	rm -f -R autom4te.cache/
fi

if [ -z $NORECONF ]; then
	echo "Running aclocal..." && \
	aclocal && \
	echo "Running autoconf..." && \
	autoconf && \
	echo "Running automake..." && \
	automake --add-missing || exit 1
fi

if [ -z $CONFIGURE ]; then echo; else
	echo "Running configure in build directory"
	mkdir -p $BUILDDIR && \
	cd $BUILDDIR && \
	../configure -C || exit 1 &&
	cd ..
fi

if [ -z $MAKE ]; then echo; else
	echo "Runnig make in build directory"
	cd $BUILDDIR && \
	make || exit 1
fi

