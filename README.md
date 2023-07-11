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

PvRecorder is an easy-to-use cross-platform audio recorder designed for real-time audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

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

For more information about the .NET demo go to [demo/dotnet](demo/dotnet).

### Go Demo

### Node.js Demo

### Rust Demo

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

## SDKs

### Python

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

Read a frame of audio:

```csharp
while (true) {
    short[] frame = recorder.Read();
    // do something with audio frame
}
```

To stop recording:

```csharp
recorder.Stop();
```

Once you are done, free the used resources. You do not have to call `Stop()` before `Dispose()`:

```csharp
recorder.Dispose();
```

To have resources freed immediately after use without explicitly calling `Dispose()`, wrap `PvRecorder` in a `using` statement:

```csharp
using (PvRecorder recorder = PvRecorder.Create(frameLength: 512)) {
    // PvRecorder usage
}
```

### Go

### Node.js

### Rust

