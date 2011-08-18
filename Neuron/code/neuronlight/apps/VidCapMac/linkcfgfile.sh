#!/bin/sh
mkdir -p DerivedData/MyRecorder/Build/Products/Debug
mkdir -p DerivedData/MyRecorder/Build/Products/Release
cd DerivedData/MyRecorder/Build/Products/Debug
ln -s ../../../../../rtenc_avc_settings.cfg
cd ../Release
ln -s ../../../../../rtenc_avc_settings.cfg
