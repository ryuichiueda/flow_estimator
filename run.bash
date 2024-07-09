#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

fix=./pgm_52_57/fixed.pgm
from=./pgm_52_57/1717995207.125090281.pgm
to=./pgm_52_57/1717995207.625553676.pgm

./a.out $fix $from $to
