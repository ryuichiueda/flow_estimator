#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

#from=./pgm_52_57/1717995178.795535617.pgm
#to=./pgm_52_57/1717995179.195682903.pgm
#
from=./pgm_52_57/1717995204.621964220.pgm
to=./pgm_52_57/1717995205.122266648.pgm

time ./a.out <(cat "$from" |sed 's/#.*//') <(cat "$to" |sed 's/#.*//') > a.pgm

convert "$from" "$to" a.pgm out.gif
convert -scale 400% -delay 100 out.gif out2.gif
