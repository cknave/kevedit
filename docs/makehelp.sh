#!/bin/sh

rm -f ../kevedit.zml

for i in *.hlp; do
	echo @@$i >> ../kevedit.zml
	cat $i >> ../kevedit.zml
done
