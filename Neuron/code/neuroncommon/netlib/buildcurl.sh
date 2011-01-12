#!/bin/sh
cd curl
cmake -G "Unix Makefiles" -DCURL_ZLIB=0 -DCURL_STATICLIB=1 -DCURL_DISABLE_LDAP=1 -DLIBRARY_OUTPUT_PATH=../..
make clean
make libcurl
