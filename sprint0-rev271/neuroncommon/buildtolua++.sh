#!/bin/sh
cd tolua++/src/lib
gcc -c *.c -I../../include -I/usr/include/lua5.1
ar rcsv libtolua++.a *.o
cd ../bin
gcc -o tolua++ tolua.c toluabind.c -I../../include -I/usr/include/lua5.1 -L../lib -ltolua++ -llua5.1
cp tolua++ ../../../../bin/tolua++
