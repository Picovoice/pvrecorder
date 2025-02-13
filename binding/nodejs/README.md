# PvRecorder Binding for Node.js

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Compatibility

- Node.js 18+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64 and arm64), and Raspberry Pi (3, 4, 5).

## Installation

```console
yarn add @picovoice/pvrecorder-node
```

## Usage

Initialize and begin recording:

```javascript
const { PvRecorder } = require("@picovoice/pvrecorder-node");

const recorder = new PvRecorder(/*frameLength*/ 512);
recorder.start()
```

(or)

Use `get_available_devices()` to get a list of available devices and then initialize the instance based on the index of a device:

```javascript

const { PvRecorder } = require("@picovoice/pvrecorder-node");

const devices = PvRecorder.getAvailableDevices()

const recorder = new PvRecorder(512, /*device index*/0);
recorder.start()
```

Read frames of audio:

```javascript
while (recorder.isRecording) {
    // const frame = recorder.readSync(), for synchronous calls
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

## Demos

[@picovoice/pvrecorder-node-demo](https://www.npmjs.com/package/@picovoice/pvrecorder-node-demo) provides command-line utilities for recording audio to a file.
