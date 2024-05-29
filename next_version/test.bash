#!/bin/bash

g++ -std=c++17 ./flow_sampling.cpp

time ./a.out <(cat ./image1.pgm |sed 's/#.*//') <(cat ./image2.pgm |sed 's/#.*//')
