# PvRecorder

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

Made in Vancouver, Canada by [Picovoice](https://picovoice.ai)

<!-- markdown-link-check-disable -->
[![Twitter URL](https://img.shields.io/twitter/url?label=%40AiPicovoice&style=social&url=https%3A%2F%2Ftwitter.com%2FAiPicovoice)](https://twitter.com/AiPicovoice)
<!-- markdown-link-check-enable -->
[![YouTube Channel Views](https://img.shields.io/youtube/channel/views/UCAdi9sTCXLosG1XeqDwLx7w?label=YouTube&style=social)](https://www.youtube.com/channel/UCAdi9sTCXLosG1XeqDwLx7w)

PvRecorder is an easy-to-use cross-platform audio recorder designed for real-time audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Table of Contents
- [PvRecorder](#pvrecorder)
  - [Table of Contents](#table-of-contents)
  - [Requirements](#requirements)
  - [Compatibility](#compatibility)
  - [Compiling](#compiling)
  - [Usage](#usage)
    - [Selecting an Audio Device](#selecting-an-audio-device)
  - [Demo](#demo)
  - [SDKs](#sdks)

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
1. Create a PvRecorder object:
```c
#include "pv_recorder.h"

const int32_t device_index = -1; // -1 == default device
const int32_t frame_length = 512;
const int32_t buffer_size_msec = 100;
const bool log_overflow = true;
const bool log_silence = false;

pv_recorder_t *recorder = NULL;
pv_recorder_status_t status = pv_recorder_init(
        device_index, 
        frame_length, 
        buffer_size_msec, 
        log_overflow, 
        log_silence, 
        &recorder);
if (status != PV_RECORDER_STATUS_SUCCESS) {
    // handle PvRecorder init error
}
```

2. Start recording audio:

```c
pv_recorder_status_t status = pv_recorder_start(recorder);
if (status != PV_RECORDER_STATUS_SUCCESS) {
    // handle PvRecorder start error
}
```

3. Read frames of audio from the recorder:
```c
// must have length equal to `frame_length` that was given to `pv_recorder_init()`
int16_t *frame = malloc(frame_length * sizeof(int16_t));

while (true) {
    pv_recorder_status_t status = pv_recorder_read(recorder, frame);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        // handle PvRecorder read error
    }
    
    // use frame of audio data
    // ...      
}
```

4. Stop recording:

```c
pv_recorder_status_t status = pv_recorder_stop(recorder);
if (status != PV_RECORDER_STATUS_SUCCESS) {
    // handle PvRecorder stop error
}
```

5. Release resources used by PvRecorder:
```c
pv_recorder_delete(recorder);
free(frame);
```

### Selecting an Audio Device

To print a list of available audio devices:
```c
char **devices = NULL;
int32_t count = 0;

pv_recorder_status_t status = pv_recorder_get_audio_devices(&count, &devices);
if (status != PV_RECORDER_STATUS_SUCCESS) {
    // handle PvRecorder get audio devices error
}

fprintf(stdout, "Printing devices...\n");
for (int32_t i = 0; i < count; i++) {
    fprintf(stdout, "index: %d, name: %s\n", i, devices[i]);
}

pv_recorder_free_device_list(count, devices);
```

The index of the device in the returned list can be used in `pv_recorder_init()` to select that device for recording.

Refer to [example/demo.c](example/demo.c) for a full example of how to use `pv_recorder` to capture audio in C.

## Demo

The executable `demo` is built as part of the compilation process.

To see the usage options for the demo:

```console
./demo
```

Run the following to see what devices are available for audio capture:

```console
./demo --show_audio_devices
```

Run the demo with the desired audio device (or -1 for the default one) and an output file path:
```console
./demo ${DEVICE_INDEX} ${OUTPUT_FILE_PATH}
```

## SDKs

Available SDKs for PvRecorder:

- [.NET](sdk/dotnet)
- [Go](sdk/go)
- [Node.js](sdk/nodejs)
- [Python](sdk/python)
- [Rust](sdk/rust)

