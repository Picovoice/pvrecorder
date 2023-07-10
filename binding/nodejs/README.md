# PvRecorder

PvRecorder Binding for Node.js.

## Compatibility

- Node.js 14+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

```console
yarn add @picovoice/pvrecorder-node
```

## Usage

Getting the list of input audio devices does not require an instance:

```javascript
const { PvRecorder } = require("@picovoice/pvrecorder-node");

const devices = PvRecorder.getAvailableDevices();
```

To start recording initialize the instance and run `start`:

```javascript
const recorder = new PvRecorder(/*sets to default device*/-1, /*frame length*/ 512);
recorder.start()
```

Get the pcm frames by calling the read function:

```javascript
while (recorder.isRecording) {
    /*const pcm = recorder.readSync(), for synchronous calls*/
    const pcm = await recorder.read();
    // do something with pcm
}
```

To stop recording just run stop on the instance:

```javascript
recorder.stop();
```

Once you are done, free the used resources. You do not have to call stop before release:

```javascript
recorder.release();
```

## Demos

[@picovoice/pvrecorder-demo](https://www.npmjs.com/package/@picovoice/pvrecorder-demo) provides command-line utilities for recording audio.
