#!/bin/bash -xv

g++ -O2 -std=c++17 ./flow_sampling.cpp
mkdir ans

tmp=/tmp/$$


ls ./pgm_52_57/??????????.?????????.pgm | sort > $tmp-files
fix=./pgm_52_57/fixed.pgm

paste $tmp-files <(sed 1,5d $tmp-files) <(sed 1,10d $tmp-files) |
while read from to verify ; do
	[ "$verify" == "" ] && continue

	time ./a.out <(cat "$from" |sed 's/#.*//') <(cat "$to" |sed 's/#.*//')  <(cat "$verify" |sed 's/#.*//') "$fix" > estimation.pgm
	convert "$from" "$to" estimation.pgm out.gif
	convert -scale 400% -delay 100 out.gif ./ans/estm.$(filename $from).gif
done

#from=./pgm_52_57/1717995207.125090281.pgm
#to=./pgm_52_57/1717995207.625553676.pgm
#verify=./pgm_52_57/1717995208.125595213.pgm
#
#time ./a.out <(cat "$from" |sed 's/#.*//') <(cat "$to" |sed 's/#.*//')  <(cat "$verify" |sed 's/#.*//') "$fix" > a.pgm
#
#convert "$from" "$to" a.pgm out.gif
#convert -scale 400% -delay 100 out.gif out2.gif

rm -f $tmp-*
