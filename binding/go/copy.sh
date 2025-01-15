#!/bin/bash

echo "Copying scripts..."
cp -r ../../resources/scripts/* ./embedded/scripts

echo "Copying Linux lib..."
cp ../../lib/linux/x86_64/libpv_recorder.so ./embedded/lib/linux/x86_64/libpv_recorder.so

echo "Copying Windows lib..."
cp ../../lib/windows/amd64/libpv_recorder.dll ./embedded/lib/windows/amd64/libpv_recorder.dll
cp ../../lib/windows/arm64/libpv_recorder.dll ./embedded/lib/windows/arm64/libpv_recorder.dll

echo "Copying macOS lib..."
cp ../../lib/mac/x86_64/libpv_recorder.dylib ./embedded/lib/mac/x86_64/libpv_recorder.dylib
cp ../../lib/mac/arm64/libpv_recorder.dylib ./embedded/lib/mac/arm64/libpv_recorder.dylib

echo "Copying RPi lib..."
cp -r ../../lib/raspberry-pi/* ./embedded/lib/raspberry-pi/
