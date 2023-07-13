# PvRecorder

[![GitHub release](https://img.shields.io/github/release/Picovoice/pvrecorder.svg)](https://github.com/Picovoice/pvrecorder/releases)
[![GitHub](https://img.shields.io/github/license/Picovoice/pvrecorder)](https://github.com/Picovoice/pvrecorder/)

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

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Table of Contents
- [PvRecorder](#pvrecorder)
  - [Table of Contents](#table-of-contents)
  - [Source Code](#source-code)
  - [Demos](#demos)
    - [Python](#python-demo)
    - [.NET](#net-demo)
    - [Go](#go-demo)
    - [Node.js](#nodejs-demo)
    - [Rust](#rust-demo)
    - [C](#c-demo)
  - [SDKs](#sdks)
    - [Python](#python)
    - [.NET](#net)
    - [Go](#go)
    - [Node.js](#nodejs)
    - [Rust](#rust)

## Source Code

If you are interested in building PvRecorder from source or integrating it into an existing C project, the PvRecorder
source code is located under the [/project](./project) directory.

## Demos

If using SSH, clone the repository with:

```console
git clone --recurse-submodules git@github.com:Picovoice/pvrecorder.git
```

If using HTTPS, clone the repository with:

```console
git clone --recurse-submodules https://github.com/Picovoice/pvrecorder.git
```

### Python Demo

Install the demo package:

```console
pip3 install pvrecorderdemo
```

To show the available audio devices run:

```console
pv_recorder_demo --show_audio_devices
```

With a working microphone connected to your device run the following in the terminal:

```console
pv_recorder_demo --output_wav_path {OUTPUT_WAV_PATH}
```

Replace `{OUTPUT_WAV_PATH}` with the path to save the audio data in `wav` format.

For more information about the Python demos go to [demo/python](demo/python).


### .NET Demo

From [demo/dotnet/PvRecorderDemo](demo/dotnet/PvRecorderDemo) run the
following in the terminal to build the demo:

```console
dotnet build
```

Make sure there is a working microphone connected to your device. From [demo/dotnet/PvRecorderDemo](demo/dotnet/PvRecorderDemo) run the
following in the terminal:

```console
dotnet run -- --output_wav_path ${OUTPUT_WAV_PATH}
```

For more information about the .NET demo, go to [demo/dotnet](demo/dotnet).

### Go Demo

The demo requires `cgo`, which on Windows may mean that you need to install a gcc compiler like [MinGW](http://mingw-w64.org) to build it properly.

From [demo/go](demo/go) run the following commands from the terminal.

```console
go run demo.go --output_wav_path {OUTPUT_WAV_PATH}
```

Replace  `{OUTPUT_WAV_PATH}` with a file path to save the audio data in `wav` format.

For more information about Go demo, go to [demo/go](demo/go).

### Node.js Demo

Install the demo package:

```console
yarn global add @picovoice/pvrecorder-node-demo
```

With a working microphone connected to your device run the following in the terminal:

```console
pvrecorder-node-demo --output_wav_path ${OUTPUT_WAV_PATH}
```

Replace `{OUTPUT_WAV_PATH}` with the file path to save the audio data in `wav` format.

For more information about NodeJS demo, go to [demo/nodejs](demo/nodejs/).

### Rust Demo

Make sure there is a working microphone connected to your device. From [demo/rust/](demo/rust) run the following in the terminal to build and run the demo:

```console
cargo run --release -- --output_wav_path ${OUTPUT_WAV_PATH}
```

For more information about the Rust demo, go to [demo/rust](demo/rust).

### C Demo

Run the following commands to build the demo app:

```console
cd demo/c
cmake -S . -B build -DPV_RECORDER_PLATFORM={PV_RECORDER_PLATFORM}
cmake --build build
```

The `{PV_RECORDER_PLATFORM}` variable will set the compilation flags for the given platform. Exclude this variable
to get a list of possible values.

Get a list of available audio recording devices:
```console
./pv_recorder_demo --show_audio_devices
```

Record to a file with a given audio device index:
```console
./pv_recorder_demo -o test.wav -d 2
```

Hit `Ctrl+C` to stop recording. If no audio device index (`-d`) is provided, the demo will use the system's default recording device.

For more information about the C demo, go to [demo/c](demo/c).

## SDKs

### Python

To start recording, initialize an instance and run `start()`:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(frame_length=512)
recorder.start()
```

Read frames of audio:

```python
while recorder.is_recording:
    frame = recorder.read()
    # process audio frame
```

To stop recording, run `stop()` on the instance:

```python
recorder.stop()
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `stop()` before `delete()`:

```python
recorder.delete()
```

For more information about the PvRecorder Python SDK, go to [binding/python](binding/python).

### .NET

Install the .NET SDK using NuGet or the dotnet CLI:

```console
dotnet add package PvRecorder
```

Initialize and begin recording:

```csharp
using Pv;

PvRecorder recorder = PvRecorder.Create(frameLength: 512);
recorder.Start();
```

Read frames of audio:

```csharp
while (recorder.IsRecording)
{
    short[] frame = recorder.Read();
    // process audio frame
}
```

To stop recording:

```csharp
recorder.Stop();
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `Stop()` before `Dispose()`:

```csharp
recorder.Dispose();
```

For more information about the PvRecorder .NET SDK, go to [binding/dotnet](binding/dotnet).

### Go

To install the PvRecorder Go module to your project, use the command:

```console
go get github.com/Picovoice/pvrecorder/binding/go
```

To start recording, initialize an instance and run `Start()`:

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

Once you are done, free the resources acquired by PvRecorder. You do not have to call `Stop()` before `Delete()`:

```go
recorder.Delete()
```

For more information about the PvRecorder Go SDK, go to [binding/go](binding/go).

### Node.js

Install Node.js binding:

```console
yarn add @picovoice/pvrecorder-node
```

To start recording, initialize the instance and run `start()`:

```javascript
const frameLength = 512;
const recorder = new PvRecorder(frameLength);
recorder.start()
```

Read frames of audio:

```javascript
while (recorder.isRecording) {
    const frame = await recorder.read();
    // process audio frame
}
```

To stop recording, call `stop()` on the instance:

```javascript
recorder.stop();
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `stop()` before `release()`:

```javascript
recorder.release();
```

For more information about the PvRecorder Node.js SDK, go to [binding/nodejs](binding/nodejs).

### Rust

Add `pv_recorder` to your app's `Cargo.toml` manifest:

```toml
[dependencies]
pv_recorder = "*"
```

To start recording, initialize the instance and run `start()`:

```rust
use pv_recorder::PvRecorderBuilder

let frame_length = 512;
let recorder = PvRecorderBuilder::new(frame_length).init()?;
recorder.start()?;
```

Read frames of audio:

```rust
while recorder.is_recording() {
    let frame = recorder.read()?;
    // process audio frame
}
```

To stop recording, run `stop()` on the instance:

```rust
recorder.stop()?;
```

For more information about the PvRecorder Rust SDK, go to [binding/rust](binding/rust).

## Releases

### v1.2.0 - July 13th, 2023

- API improvements
- Improved docs
- Added unit tests and actions for each SDK
- Addressed race condition when stop is called during a read
- Fixed .NET version support
- Demos write to WAV files

### v1.1.0 - November 10th, 2022

- Added logs that warn users if recorded audio is silent for a few number of seconds.
