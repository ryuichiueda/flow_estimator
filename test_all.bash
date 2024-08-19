#!/bin/bash -xv

g++ -O2 -std=c++17 evaluation.cpp -o evaluation
g++ -O2 -std=c++17 ./flow_sampling.cpp
mkdir -p ans

dir=./2024_07_30-15_39_42
tmp=/tmp/$$
fix=$dir/fixed.pgm
after=10
skip=1

ls $dir/??????????.?????????.pgm |
sort                                    |
tr '\n' ' '                             |
awk '{for(i=1;i<=NF;i++){for(j=i;j<=NF;j++)printf($j" "); print ""}}' |
awk -v t=$after -v s=$skip 'NF>=5+4*s+t{print $1,$(1+s),$(1+2*s),$(1+3*s),$(1+4*s),$(1+4*s+t)}'  |
tee list.txt |
while read a b c d e ref ; do
	time ./a.out $after $skip $fix $a $b $c $d $e > ./ans/estm.$(basename $ref) || { rm -f ./ans/estm.$(basename $ref) ; continue ;}
	convert $a $b $c $d $e ./ans/estm.$(basename $ref) ./ans/out.gif || { rm -f ./ans/out.gif ; continue ;}
	convert -scale 400% -delay 100 ./ans/out.gif ./ans/estm.$(basename $ref).gif
	./evaluation $fix $ref ./ans/estm.$(basename $ref) > ./ans/out.ppm 2> ./ans/$(basename $ref).result.txt
	convert -scale 400% ./ans/out.ppm ./ans/diff.$(basename $ref).ppm
done |& tee - log.txt
