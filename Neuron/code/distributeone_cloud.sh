#!/bin/sh

echo "USAGE: ./distributeone_cloud.sh <username@ipaddress> < pemfile>"
echo "Distributing to $1 all binaries, licenses, profiles, and videos."

cd bin
ssh -i $2 $1 "rm bin >/dev/null 2>&1 ; mkdir bin >/dev/null 2>&1"
scp -i $2 sf $1:bin/
scp -i $2 ubrain $1:bin/

scp -i $2 $NDDSHOME/rti_license.dat $1:bin/
scp -i $2 $NDDSHOME/rti_license.dat $1:

scp -i $2 ../tools/DemoEndpoint/USER_QOS_PROFILES.xml $1:bin/
scp -i $2 ../tools/DemoEndpoint/USER_QOS_PROFILES.xml $1:

ssh -i $2 $1 "mkdir Videos >/dev/null 2>&1"
scp -i $2 ~/Videos/* $1:Videos/
