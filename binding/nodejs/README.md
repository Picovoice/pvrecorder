# PvRecorder Binding for Node.js

## Compatibility

- Node.js 14+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (2, 3, 4), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

```console
yarn add @picovoice/pvrecorder-node
```

## Usage

To start recording initialize the instance and run `start`:

```javascript
const { PvRecorder } = require("@picovoice/pvrecorder-node");

const recorder = new PvRecorder(/*frame length*/ 512);
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

Get a frame of audio by calling the read function:

```javascript
while (recorder.isRecording) {
    // const frame = recorder.readSync(), for synchronous calls
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

## Demos

[@picovoice/pvrecorder-demo](https://www.npmjs.com/package/@picovoice/pvrecorder-demo) provides command-line utilities for recording audio to a file.
