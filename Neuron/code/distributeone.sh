#!/bin/sh

echo "Distributing to $1 all binaries, licenses, profiles, and videos."

cd bin
scp sf $1:bin/
scp demoendpoint $1:bin/
scp ubrain $1:bin/

scp $NDDSHOME/rti_license.dat $1:bin/
scp $NDDSHOME/rti_license.dat $1:

scp ../tools/DemoEndpoint/USER_QOS_PROFILES.xml $1:bin/
scp ../tools/DemoEndpoint/USER_QOS_PROFILES.xml $1:

ssh $1 "mkdir Videos >/dev/null 2>&1"
scp ~/Videos/* $1:Videos/
