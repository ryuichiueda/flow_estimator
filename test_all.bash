#!/bin/bash -xv

g++ -O2 -std=c++17 ./flow_sampling.cpp
mkdir -p ans

tmp=/tmp/$$
fix=./pgm_52_57/fixed.pgm

ls ./pgm_52_57/??????????.?????????.pgm |
sort                                    |
tr '\n' ' '                             |
awk '{for(i=1;i<=NF;i++){for(j=i;j<=NF;j++)printf($j" "); print ""}}' |
awk 'NF>=10{print $1,$2,$3,$4,$5,$10}'  |
while read a b c d e ref ; do
	time ./a.out $fix $a $b $c $d $e > ./ans/estm.$(basename $ref)
	convert $a $b $c $d $e ./ans/estm.$(basename $ref) ./ans/out.gif
	convert -scale 400% -delay 100 ./ans/out.gif ./ans/estm.$(basename $ref).gif
done
