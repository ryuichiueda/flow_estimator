# flow_estimator

Output a one step future histogram estimated from two sequential ones.

## Quick Start

```bash
### compile (c++17 can be changed to new one) ###
$ g++ -std=c++17 flow_estimator.cpp
### run (run.bash eliminates comments from PGM files and kicks ./a.out) ###
$ ./run.bash two_step_before.pgm one_step_before.pgm
P2
10 10
255
  10   0   0   0   0   0   0   0   0   0
   0   0   0   0   0   0   0   0   0   0
   0   0   0   0   0   0   0   0   0   0
   0   0   0   0   0   0   0   0   0   0
   0   0   0  10  10   0   0   0   0   0
   0   0  10  15  40   5   0   0   0   0
   0   0  30  65  25  20   0   0   0   0
   0   0   0   5   5   0   0   0   0   0
   0   0   0   0   0   0   0   0   0   0
   0   0   0   0   0   0   0   0   0   0
```

## I/O

### Onput

The cpp program accepts two files via its arguments:
* 1st file: the occupancy grid map obtained two step before
    * [example](./two_step_before.pgm)
* 2nd file: the occupancy grid map obtained one step before
    * [example](./one_step_before.pgm)

The occupancy grid map should be written in a text PGM (P2) file.

#### examples

* png images converted from the pgm files:

![](./doc/before.png)
![](./doc/after.png)

### Output

The cpp program outputs the following data from its stdout:
* a forecasted occupancy grid map as a text PGM format.

### example

* png image converted from the output:

![](./doc/estimation.png)

* gif image built from the above images:

![](./doc/example.gif)

## TODO

* To use better random variables
* To make users can program their own estimator in the cpp file.

## Acknowledgment

This work was supported by JSPS KAKENHI Grant Number JP24K15127.
