#!/bin/bash

echo "Copying scripts..."
cp -r ../../scripts ./scripts

echo "Copying Linux lib..."
cp ../../lib/linux/x86_64/libpv_recorder.so ./lib/linux/x86_64/libpv_recorder.so

echo "Copying Windows lib..."
cp ../../lib/windows/amd64/libpv_recorder.so ./lib/windows/amd64/libpv_recorder.dll

echo "Copying macOS lib..."
cp ../../lib/macOS/x86_64/libpv_recorder.so ./lib/macOS/x86_64/libpv_recorder.dylib

echo "Copying RPi lib..."
cp -r ../../lib/raspberry-pi/* ./lib/raspberry-pi/

echo "Copying Jetson lib..."
cp ../../lib/jetson/cortex-a57-aarch64/libpv_recorder.so ./lib/jetson/cortex-a57-aarch64/libpv_recorder.so

echo "Copying BeagleBone lib..."
cp ../../lib/beaglebone/libpv_recorder.so ./lib/beaglebone/libpv_recorder.so
