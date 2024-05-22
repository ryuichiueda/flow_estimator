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

## Input and Output

### Input

The cpp program accepts two files via its arguments:
* 1st file: the occupancy grid map obtained two step before
* 2nd file: the occupancy grid map obtained one step before

The occupancy grid map should be written in a text PGM (P2) file.

### Output

The cpp program outputs the following data from its stdout:
* a forecasted occupancy grid map as a text PGM format.

## TODO

* To consider fixed obstacles (Two input PGM should be compared at first.)
* To use better random variables
