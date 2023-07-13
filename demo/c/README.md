# PvRecorder Demo for C

This project contains a C command-line demo for PvRecorder that demonstrates how to use PvRecorder to record audio to a WAV file.

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Requirements

- CMake 3.4+.
- C99 compatible compiler.
- **Windows**: MinGW.

## Compatibility

- Linux (x86_64)
- macOS (x86_64, arm64)
- Windows (amd64)
- Raspberry Pi (Zero, 2, 3 and 4)
- NVIDIA Jetson Nano
- BeagleBone

## Compiling

Run the following commands to build the demo app:

```console
git submodule update --init --recursive
cmake -S . -B build -DPV_RECORDER_PLATFORM={PV_RECORDER_PLATFORM}
cmake --build build
```

The `{PV_RECORDER_PLATFORM}` variable will set the compilation flags for the given platform. Exclude this variable
to get a list of possible values.

## Usage

To see the usage options for the demo:
```console
./pv_recorder_demo
```

Get a list of available audio recording devices:
```console
./pv_recorder_demo --show_audio_devices
```

Record to a file with a given audio device index:
```console
./pv_recorder_demo -o test.wav -d 2
```

Hit `Ctrl+C` to stop recording. If no audio device index (`-d`) is provided, the demo will use the system's default recording device.
