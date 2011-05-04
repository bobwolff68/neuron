rm CMakeCache.txt
cmake -G "Unix Makefiles"
make clean
make
cp bin/h264source_standalone ~/

