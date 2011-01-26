#!/bin/sh

sh distributeone.sh rwolff@207.145.121.125
sh distributeone.sh rwolff@192.168.46.30
sh distributeone.sh manjesh@192.168.46.74
sh distributeone.sh rwolff@192.168.46.73
#sh distributeone.sh rwolff@192.168.46.75
sh distributeone_cloud.sh ec2-user@50.18.56.81 ~/.ssh/xvd-demo.pem
sh distributeone_cloud.sh ec2-user@50.18.56.238 ~/.ssh/xvd-demo.pem
