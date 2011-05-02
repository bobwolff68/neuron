#!/bin/sh
# If cppunit library is already present, we're good to go.
# if it doesn't exist, build and install it (requires sudo privs)
if test -e /usr/local/lib/libcppunit.a
then
 echo $0: libcppunit.a already exists. SKIPPING BUILD.
 exit 1
fi

cd cppunit
./configure
make clean
make
sudo make install
