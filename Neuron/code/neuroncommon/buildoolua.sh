#!/bin/sh

# ensure gmock exists if we are going to test etc. Else we can just make 'oolua' and stop.

# ensure premake4 exists
# if it doesn't exist, error out.
if ! test -e ./premake4; then
 echo $0: Failure: premake4 was not found. It should have come out of the repo.
 exit 1
elif ! test -x ./premake4; then
 chmod +x ./premake4
fi

export CPATH=/usr/include/lua5.1
cd oolua
../premake4 gmake linux
make config=debug oolua
make config=release oolua

# copy .a libraries up to n
cp bin/Debug/* ../../lib/
cp bin/Release/* ../../lib/

# copy include files to neuron's code/include/oolua/
if ! test -d ../../include/oolua
then
  if test -e ../../include/oolua then
    echo $0: Failure: Cannot copy include files to ../include/tolua as there is not a directory.
  else
    mkdir ../../include/oolua
  fi
fi
cp include/* ../../include/oolua/

