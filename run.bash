#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

fix=./pgm_52_57/fixed.pgm
a=./pgm_52_57/1717995178.795535617.pgm
b=./pgm_52_57/1717995178.895143492.pgm
c=./pgm_52_57/1717995178.995259531.pgm
d=./pgm_52_57/1717995179.095244220.pgm
e=./pgm_52_57/1717995179.195682903.pgm

time ./a.out $fix $a $b $c $d $e > out.pgm

convert $a $b $c $d $e out.pgm out.gif
convert -scale 400% -delay 100 out.gif out2.gif
