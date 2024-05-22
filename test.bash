#!/bin/bash

g++ -std=c++17 ./flow_estimator.cpp

time ./a.out <(cat ./image1.pgm |sed 's/#.*//') <(cat ./image2.pgm |sed 's/#.*//')
