#!/bin/sh

# Make bin/ and lib/
mkdir -p bin
mkdir -p lib

if test $1 = "clean"
then
  CLEAN=1
  rm lib/*
  rm neuroncommon/netlib/libcurl.a
fi

# Now make the qp libraries and copy the resultant .a files into Neuron/code/lib
cd ../../Common/qp/ports/80x86/linux/gnu
if test $CLEAN -eq 1
then
  echo "cleaning..."
  make dbg_clean
  make rel_clean
  make spy_clean
fi
echo "making..."

make dbg
make rel
# cannot do this yet - make spy
echo "Skipping make of 'spy' - technical issues to solve."

cp rel/*.a ../../../../../../Neuron/code/lib/
cp dbg/libqep.a ../../../../../../Neuron/code/lib/libqep_d.a
cp dbg/libqf.a ../../../../../../Neuron/code/lib/libqf_d.a

cd ../../../../../../Neuron/code/neuroncommon

# Only re-build cppunit if needed. It is a shared-library item so it should be built once
# per time of revision publishing.
# Newer script automatically decides to make or not make/install cppunit.
sh ./buildcppunit.sh

# Make sure lua and liblua/dev are installed.
if ! which lua
then
  sudo apt-get install lua5.1
fi

# now validate the include files are ready.
if ! test -e /usr/include/lua5.1/lua.h
then
  sudo apt-get install liblua5.1-0-dev
fi
# Very simple/fast recompile for lua bindings from C/C++ to Lua functions and classes
# Unnecessary as of April 27th. Replaced for now with OOLUA. sh ./buildtolua++.sh

# Very simple/fast recompile for lua bindings from C/C++ to Lua functions and classes
sh ./buildoolua.sh

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
$NDDSHOME/scripts/rtiddsgen -replace -namespace media.idl

cd ../entity_info_idl
$NDDSHOME/scripts/rtiddsgen -replace -namespace entityinfo.idl

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

