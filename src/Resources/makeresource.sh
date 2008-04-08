#!/bin/bash

NAMES=`ls *.ico *.ICO`

for i in $NAMES ; do
	./makestring.sh $i
done