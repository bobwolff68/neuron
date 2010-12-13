#!/bin/sh
cd curl
cmake -G "Unix Makefiles" -DCURL_ZLIB=0 -DCURL_STATICLIB=1 -DLIBRARY_OUTPUT_PATH=../..
make clean
make libcurl
cp curl/lib/libcurl.a .
