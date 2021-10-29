# PV_Recorder

A cross platform audio recorder that captures single-channel audio at a sample rate of 16kHz.

## Requirements

- Node.js 12+

## Compatibility

- Windows (x86_64)
- macOS
    - x86_64
    - arm64
- Linux (x86_64)
- Raspberry Pi:
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Installation

```console
yarn add @picovoice/pvrecorder-node
```

## Usage

Getting the list of input devices does not require an instance:

```javascript
const PvRecorder = require("@picovoice/pvrecorder-node");

const devices = PvRecorder.getAudioDevices();
```

To start recording initialize the instance and run `start`:

```javascript
const recorder = new PvRecorder(/*sets to default device*/-1, /*frame length*/ 512);
recorder.start()
```

Get the pcm frames by calling the read function:

```javascript
while (true) {
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

```csharp
recorder.release()
```

### Demo

For more detailed information on how to use the pv_recorder Node.js sdk, see [demo/demo.js](demo/demo.js). 

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{RAW_OUTPUT_PATH}` as the path to save the raw audio data 

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{RAW_OUTPUT_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
node demo/demo.js --show_audio_devices
```

To run the audio recorder:

```console
node demo/demo.js --audio_device_index {AUDIO_DEVICE_INDEX} --raw_output_path {RAW_OUTPUT_PATH}
```
