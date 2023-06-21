# PvRecorder Project

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
after a successful compilation. `{OUTPUT_DIR}` should be a directory **relative** to the [lib](../lib) directory.

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

Refer to [pv_recorder_demo.c](../demo/c/pv_recorder_demo.c) for a full example of how to use `pv_recorder` to capture audio in C.
