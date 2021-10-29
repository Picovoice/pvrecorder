# PV_Recorder

A cross platform audio recorder to read one channel and 16kHz samples.

## Requirements

- Go 1.16+

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

## Installation

```console
go get github.com/Picovoice/pvrecorder/sdk/go
```

## Usage

To get the list of available devices:

```go
import . "github.com/Picovoice/pvrecorder/sdk/go"

devices, err := GetAudioDevices()
if err != nil {
    // error
}
```

To start recording, initialize the instance and run start function:

```go
import . "github.com/Picovoice/pvrecorder/sdk/go"

recorder := PvRecorder{
    DeviceIndex: -1, // Using -1 for index uses default audio input device.
    FrameLength: 512,
    BufferSizeMSec: 1000,
    LogOverflow: 1,
}
if err := recorder.Init(); err != nil {
    // error
}
if err := recorder.Start(); err != nil {
    // error
}
```

To read the pcm frames, run:

```go
pcm, err := recorder.Read()
if err != nil {
    // handle error
}
// do something with pcm
```

To stop recording just run stop on the instance:

```go
recorder.Stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```go
recorder.Delete()
```

### Demo

For more detailed information on how to use the pv_recorder go sdk, please that a look at [demo/demo.go](demo/demo.go). 

To run the demo:

```console
go run demo/demo.go
```