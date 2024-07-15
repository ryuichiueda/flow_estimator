#!/bin/bash -xv

fix=./pgm_52_57/fixed.pgm
ans=./pgm_52_57/1717995179.996512086.pgm
estimation=./ans/estm.1717995179.996512086.pgm

g++ -O2 -std=c++17 evaluation.cpp -o evaluation

./evaluation $fix $ans $estimation
