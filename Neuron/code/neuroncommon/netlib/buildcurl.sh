#!/bin/sh
# ensure premake4 exists
# if it doesn't exist, error out.
if test -e ./libcurl.a
then
 echo $0: LIBCURL: libcurl.a already found. SKIPPING BUILD.
 exit 1
fi

cd curl
cmake -G "Unix Makefiles" -DCURL_ZLIB=0 -DCURL_STATICLIB=1 -DCURL_DISABLE_LDAP=1 -DLIBRARY_OUTPUT_PATH=../..
make clean
make libcurl
