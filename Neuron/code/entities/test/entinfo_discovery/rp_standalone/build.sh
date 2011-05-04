rm CMakeCache.txt
cmake -G "Unix Makefiles"
make clean
make
cp bin/rp_standalone ~/
