#!/bin/bash
grep 'L' ../src/*.cpp ../src/*.h | awk '{gsub(/"/,"\n");print}' | awk 'BEGIN{f=0}{if (f) { print "[" $0 "]"; f=0; }else if ( $0 ~ /L[(]$/ || $0 ~ /L[(]L$/ ) f=1;}' | sort -f > original.tmp
for i in ../src/Resources/lang/*.vpl; do
    itmp=`basename $i`
    iconv -f ucs-2 -t utf-8 $i -o ${itmp}.before.utf8
    dos2unix ${itmp}.before.utf8
    cat original.tmp ${itmp}.before.utf8 | gawk '
	BEGIN{
	    FS="\\][^\\[]\\["
	}
	{
	    sub(/^[^\[]*\[/, ""); 
	    sub(/\]$/, ""); 
	    if ($1 != "") a[$1] = $2;
	}
	END{
	    n = asorti(a, b);
	    for (i = 1; i <= n; ++i) if (a[b[i]] != "") print "[" b[i] "]\t[" a[b[i]] "]"; 
	    for (i = 1; i <= n; ++i) if (a[b[i]] == "") print "[" b[i] "]";
	}' > ${itmp}.after.utf8
    unix2dos ${itmp}.after.utf8
    iconv -f utf-8 -t ucs-2 ${itmp}.after.utf8 -o ${itmp}
done
rm *.vpl.*