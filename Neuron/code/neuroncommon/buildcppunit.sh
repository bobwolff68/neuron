#!/bin/sh
cd cppunit
./configure
make clean
make
sudo make install
