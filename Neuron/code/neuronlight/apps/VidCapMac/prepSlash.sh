#!/bin/sh
echo
echo "Preparing '/' directory for demo of application. Will require sudo password to continue."
echo
RETDIR=$PWD

MYCFG=$PWD/rtenc_avc_settings.cfg

cd /

for i in 0 1 2 3 4 5 6 7 8 9
do
  sudo rm -f stream$i*
  sudo mkfifo stream$i
  sudo chmod 666 stream$i

  sudo mkfifo stream$i.264
  sudo chmod 666 stream$i.264

  sudo mkfifo stream$i.aac
  sudo chmod 666 stream$i.aac

  sudo mkfifo stream$i.mp3
  sudo chmod 666 stream$i.mp3

  sudo mkfifo stream$i.mpa
  sudo chmod 666 stream$i.mpa
done

sudo rm -f rtenc_avc_settings.cfg
sudo ln -s $MYCFG
sudo chmod 777 rtenc_avc_settings.cfg

cd $RETDIR
