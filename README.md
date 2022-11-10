# PV_Recorder

[![GitHub release](https://img.shields.io/github/release/Picovoice/pvrecorder.svg)](https://github.com/Picovoice/pvrecorder/releases)
[![GitHub](https://img.shields.io/github/license/Picovoice/pvrecorder)](https://github.com/Picovoice/pvrecorder/)
[![GitHub language count](https://img.shields.io/github/languages/count/Picovoice/pvrecorder)](https://github.com/Picovoice/pvrecorder/)

<!-- markdown-link-check-disable -->
[![PyPI](https://img.shields.io/pypi/v/pvrecorder)](https://pypi.org/project/pvrecorder/)
[![Nuget](https://img.shields.io/nuget/v/pvrecorder)](https://www.nuget.org/packages/pvrecorder/)
[![Go Reference](https://pkg.go.dev/badge/github.com/Picovoice/pvrecorder/binding/go.svg)](https://pkg.go.dev/github.com/Picovoice/pvrecorder/binding/go)
[![npm](https://img.shields.io/npm/v/@picovoice/pvrecorder-node?label=npm%20%5Bnode%5D)](https://www.npmjs.com/package/@picovoice/pvrecorder-node)
[![Crates.io](https://img.shields.io/crates/v/pv_recorder)](https://crates.io/crates/pv_recorder)
<!-- markdown-link-check-enable -->

## Requirements

- CMake 3.4+.
- C99 compatible compiler.
- **Windows**: MinGW.

## Compatibility

- Windows (amd64)
- macOS 
    - x86_64
    - arm64
- Linux (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Compiling

The variable `{OUTPUT_DIR}` will be used to select the directory to copy the shared object
after a successful compilation. `{OUTPUT_DIR}` should be a directory **relative** to the [lib](lib) directory.

Run the following commands to build and test (`{OUTPUT_DIR}` can be empty if you wish not to copy):

```console
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DOUTPUT_DIR={OUTPUT_DIR}
cmake --build .
```

## Usage

Refer to [example/demo.c](example/demo.c) for how to use `pv_recorder` to capture audio in C.

The executable `demo` is also built together while compiling.

To get the audio devices run:

```console
./demo --show_audio_devices
```

We will refer to `{DEVICE_INDEX}` as the index of the audio device of your choice and refer to 
`{OUTPUT_FILE_PATH}` as the path of the filepath to save the audio file in wav format. To run the demo and save
the raw file, run:

```console
./demo {DEVICE_INDEX} {OUTPUT_FILE_PATH}
```

## SDK

Checkout the available SDKs for pvrecorder:

- [.NET](sdk/dotnet)
- [Go](sdk/go)
- [Node.js](sdk/nodejs)
- [Python](sdk/python)
- [Rust](sdk/rust)

