#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

fix=./pgm_52_57/fixed.pgm
from=./pgm_52_57/1717995178.795535617.pgm
to=./pgm_52_57/1717995178.895143492.pgm

time ./a.out $fix $from $to
