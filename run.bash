#!/bin/bash

./a.out <(cat $1 |sed 's/#.*//') <(cat $2 |sed 's/#.*//')
