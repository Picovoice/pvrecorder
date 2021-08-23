# PV_Recorder

A cross platform audio recorder to read one channel and 16kHz samples.

## Requirements

- Go 1.16+

## Compatibility

- Windows (amd64)
- macOS (x86_64)
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

Before recording, create a callback that processes the pcm:

```go
func callback(pcm []int16) {
    // do something with pcm
}
```

To start recording, initialize the instance and run start function:

```go
import . "github.com/Picovoice/pvrecorder/sdk/go"

recorder := PVRecorder{
    DeviceIndex: -1,
    FrameLength: 512,
    Callback: callback,
}
if err := recorder.Init(); err != nil {
    // error
}
if err := recorder.Start(); err != nil {
    // error
}
```

To stop recording just run stop on the instance:

```go
recorder.Stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```go
recorder.Delete()
```