#!/bin/sh

cd neuroncommon/netlib
sh ./buildcurl.sh

cd ..
cmake -G "Unix Makefiles"
make clean
make

cd ../control
cmake -G "Unix Makefiles"
make clean
make

cd ../media/idl
rtiddsgen -replace -namespace media.idl

cd ../../sf
cmake -G "Unix Makefiles"
make clean
make

cd ../tools/DemoEndpoint
cmake -G "Unix Makefiles"
make clean
make

cd ../ubrain
cmake -G "Unix Makefiles"
make clean
make
echo "Completed building all binaries."

