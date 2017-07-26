#!/bin/sh

SRCDIR=.
OUTDIR=..

if [ $# -gt 0 ]; then
	SRCDIR=$1
fi

if [ $# -gt 1 ]; then
	OUTDIR=$2
fi

HELPFILE=$OUTDIR/kevedit.zml
SRCFILES=$SRCDIR/*.hlp

rm -f $HELPFILE

for i in $SRCFILES; do
	basename=`basename $i` &&
	echo @@$basename >> $HELPFILE &&
	cat $i >> $HELPFILE || exit 1
done

echo "Help metafile $HELPFILE generated from $SRCFILES"
