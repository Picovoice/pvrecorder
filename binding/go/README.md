# PvRecorder Binding for Go

## Compatibility

- Go 1.16+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

```console
go get github.com/Picovoice/pvrecorder/binding/go
```

## Usage

To get the list of available devices:

```go
import . "github.com/Picovoice/pvrecorder/binding/go"

devices, err := GetAvailableDevices()
if err != nil {
    // error
}
```

To start recording, initialize the instance and run start function:

```go
import . "github.com/Picovoice/pvrecorder/binding/go"

recorder := NewPvRecorder(/*FrameLength*/512)
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

For more detailed information on how to use the pv_recorder go binding, please that a look at [demo/demo.go](../../demo/go/demo.go). 
