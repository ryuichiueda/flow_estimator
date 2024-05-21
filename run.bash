#!/bin/bash

g++ ./flow_estimator.cpp

./a.out <(cat $1 |sed 's/#.*//') <(cat $2 |sed 's/#.*//')
