#!/bin/sh

HELPFILE=../kevedit.zml

rm -f $HELPFILE

for i in *.hlp; do
	echo @@$i >> $HELPFILE
	cat $i >> $HELPFILE
done

echo "Help metafile generated"
