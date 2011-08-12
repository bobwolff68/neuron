#!/bin/sh
cd ../../../../../Common/live555
./genMakefiles macosx-32bit
make clean
make
