# PvRecorder Binding for Go

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Compatibility

- Go 1.16+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

```console
go get github.com/Picovoice/pvrecorder/binding/go
```

## Usage

Initialize and begin recording:

```go
import . "github.com/Picovoice/pvrecorder/binding/go"

recorder = NewPvRecorder(/*FrameLength*/512)
recorder.Init()
if err != nil {
    // handle init error
}
defer recorder.Delete()

err = recorder.Start()
if err != nil {
    // handle start error
}
```

(or)

Use `GetAvailableDevices()` to get a list of available devices and then initialize the instance based on the index of a device:

```go
import . "github.com/Picovoice/pvrecorder/binding/go"

devices = GetAvailableDevices() // select index of device

recorder = NewPvRecorder(/*FrameLength*/512)
recorder.DeviceIndex = 0
recorder.Init()
if err != nil {
    // handle init error
}
defer recorder.Delete()

err = recorder.Start()
if err != nil {
    // handle start error
}
```

Get a frame of audio by calling the `Read()` function:

```go
frame, err := recorder.Read()
if err != nil {
    // handle error
}
```

To stop recording, call `Stop()` on the instance:

```go
recorder.Stop()
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `stop()` before `delete()`:

```go
recorder.Delete()
```

### Demo

For more detailed information on how to use the PvRecorder Go binding, please that a look at [demo/demo.go](../../demo/go/demo.go).
