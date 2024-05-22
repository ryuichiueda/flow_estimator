#!/bin/bash

g++ -std=c++17 ./flow_estimator.cpp

time ./a.out <(cat ./two_step_before.pgm |sed 's/#.*//') <(cat ./one_step_before.pgm |sed 's/#.*//')
