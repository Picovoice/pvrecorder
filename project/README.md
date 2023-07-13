# PvRecorder Source Project

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

Run the following commands to build and test (`{OUTPUT_DIR}` can be empty if you wish not to copy):

```console
git submodule update --init --recursive
cmake -S . -B build -DOUTPUT_DIR={OUTPUT_DIR} -DPV_RECORDER_PLATFORM={PV_RECORDER_PLATFORM}
cmake --build build
```

The variable `{OUTPUT_DIR}` will be used to select the directory to copy the shared object
after a successful compilation. `{OUTPUT_DIR}` should be a directory **relative** to the [lib](../lib) directory.

The `{PV_RECORDER_PLATFORM}` variable will set the compilation flags for the given platform. Exclude this variable
to get a list of possible values.

## Usage

1. Create a PvRecorder object:
```c
#include "pv_recorder.h"

const int32_t frame_length = 512;
const int32_t device_index = -1; // -1 == default device
const int32_t buffered_frame_count = 10;

pv_recorder_t *recorder = NULL;
pv_recorder_status_t status = pv_recorder_init(
        frame_length,
        device_index,
        buffered_frame_count,
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
char **device_list = NULL;
int32_t device_list_length = 0;

pv_recorder_status_t status = pv_recorder_get_available_devices(&device_list_length, &device_list);
if (status != PV_RECORDER_STATUS_SUCCESS) {
    // handle PvRecorder get audio devices error
}

fprintf(stdout, "Printing devices...\n");
for (int32_t i = 0; i < device_list_length; i++) {
    fprintf(stdout, "index: %d, name: %s\n", i, device_list[i]);
}

pv_recorder_free_available_devices(device_list_length, device_list);
```

The index of the device in the returned list can be used in `pv_recorder_init()` to select that device for recording.

Refer to [pv_recorder_demo.c](../demo/c/pv_recorder_demo.c) for a full example of how to use `pv_recorder` to capture audio in C.
