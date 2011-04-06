#!/bin/sh
ssh rwolff@192.168.46.118 "killall sf"
ssh rwolff@192.168.46.30 "killall sf"
ssh rwolff@192.168.46.78 "killall sf"
ssh rwolff@192.168.46.73 "killall sf"
ssh -i ~/.ssh/xvd-demo.pem ec2-user@50.18.56.81 "killall sf"
