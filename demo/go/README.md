# PvRecorder Go Demo

## Requirements

- go 1.16+
- **Windows**: The demo requires `cgo`, which means that you need to install a gcc compiler like [Mingw](http://mingw-w64.org/) to build it properly.

## Compatibility

- Linux (x86_64)
- macOS (x86_64, arm64)
- Windows (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Usage

NOTE: The working directory for the following go commands is:

```console
pvrecorder/demo/go
```

### Demo

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{OUTPUT_PATH}` as the path to save the audio data in `wav` format.

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{OUTPUT_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
go run demo.go --show_audio_devices
```

To run PvRecorder run:

```console
go run demo.go --audio_device_index {AUDIO_DEVICE_INDEX} --output_path {OUTPUT_PATH}
```
