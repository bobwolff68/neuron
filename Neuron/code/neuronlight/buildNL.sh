#!/bin/sh

#build capture

#build network
cd network/build
make clean
rm CMakeCache.txt
cmake ../
make
