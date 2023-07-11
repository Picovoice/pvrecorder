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

### Go Demo

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

For more information about NodeJS demos go to [demo/nodejs](demo/nodejs/).

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

To start recording initialize an instance and run start:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(frame_length=512)
recorder.start()
```

Get the frames by calling the read function:

```python
frame = recorder.read()
```

To stop recording just run stop on the instance:

```python
recorder.stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```python
recorder.delete()
```

### .NET

### Go

### Node.js

Install Node.js binding:

```console
yarn add @picovoice/pvrecorder-node
```

To start recording initialize the instance and run `start`:

```javascript
const recorder = new PvRecorder(512);
recorder.start()
```

Get a frame of audio by calling the read function:

```javascript
while (recorder.isRecording) {
    const frame = await recorder.read();
    // do something with frame
}
```

To stop recording, run stop on the instance:

```javascript
recorder.stop();
```

Once you are done, free the used resources. You do not have to call stop before release:

```javascript
recorder.release();
```

### Rust

