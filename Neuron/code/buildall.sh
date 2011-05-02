#!/bin/sh

cd neuroncommon

# Only re-build cppunit if needed. It is a shared-library item so it should be built once
# per time of revision publishing.
# sh ./buildcppunit

cd netlib
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
