#!/bin/sh

make -C ..
make clean
make
./tests
