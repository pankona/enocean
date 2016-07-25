#!/bin/bash -eu

gcc -c -O3 serialread.c
gcc -c -O3 main.c
gcc -o test main.o serialread.o
