#!/bin/bash

echo "Copying scripts..."
cp -r ../../scripts/* ./embedded/scripts

echo "Copying Linux lib..."
cp ../../lib/linux/x86_64/libpv_recorder.so ./embedded/lib/linux/x86_64/libpv_recorder.so

echo "Copying Windows lib..."
cp ../../lib/windows/amd64/libpv_recorder.dll ./embedded/lib/windows/amd64/libpv_recorder.dll

echo "Copying macOS lib..."
cp ../../lib/mac/x86_64/libpv_recorder.dylib ./embedded/lib/mac/x86_64/libpv_recorder.dylib
cp ../../lib/mac/arm64/libpv_recorder.dylib ./embedded/lib/mac/arm64/libpv_recorder.dylib

echo "Copying RPi lib..."
cp -r ../../lib/raspberry-pi/* ./embedded/lib/raspberry-pi/

echo "Copying Jetson lib..."
cp ../../lib/jetson/cortex-a57-aarch64/libpv_recorder.so ./embedded/lib/jetson/cortex-a57-aarch64/libpv_recorder.so

echo "Copying BeagleBone lib..."
cp ../../lib/beaglebone/libpv_recorder.so ./embedded/lib/beaglebone/libpv_recorder.so
