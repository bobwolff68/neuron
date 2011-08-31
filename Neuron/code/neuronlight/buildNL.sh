#!/bin/sh

mkdir -p ../lib

#build capture
cd capture/build
make clean
rm CMakeCache.txt
cmake ../
make
cd ../..
cp capture/build/lib/libcapture.a ../lib/

#build codec
cd codec/build
make clean
rm CMakeCache.txt
cmake ../
make
cd ../..
cp codec/build/lib/libcodec.a ../lib/

#build network
cd network/build
make clean
rm CMakeCache.txt
cmake ../
make
cd ../..
cp network/build/lib/libnetwork.a ../lib/
