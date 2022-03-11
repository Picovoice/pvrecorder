# PV_Recorder

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
after a successful compilation. `{OUTPUT_DIR}` should be a directory **relative** to the [lib](lib/) directory.

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

- [.NET](/sdk/dotnet)
- [Go](/sdk/go)
- [Node.js](/sdk/nodejs)
- [Python](/sdk/python)
- [Rust](/sdk/rust)

