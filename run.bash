#!/bin/bash

g++ ./flow_estimator.cpp

time ./a.out <(cat $1 |sed 's/#.*//') <(cat $2 |sed 's/#.*//')
