#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

fix=./pgm_52_57/fixed.pgm
a=./pgm_52_57/1717995178.795535617.pgm
b=./pgm_52_57/1717995178.895143492.pgm
c=./pgm_52_57/1717995178.995259531.pgm

time ./a.out $fix $a $b $c
