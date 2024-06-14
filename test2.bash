#!/bin/bash

g++ -O2 -std=c++17 ./flow_sampling.cpp

time ./a.out <(cat ./pgm_52_57/1717995178.795535617.pgm |sed 's/#.*//') <(cat ./pgm_52_57/1717995179.195682903.pgm |sed 's/#.*//')
