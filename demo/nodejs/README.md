# PvRecorder Demo for Node.js

## Compatibility

- Node.js 14+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (2, 3, 4), NVIDIA Jetson (Nano), and BeagleBone.

For more detailed information on how to use the pv_recorder Node.js sdk, see [demo.js](../../demo/nodejs/demo.js).

## Installation:

To install the demos and make them available on the command line, use either of the following `yarn` or `npm` commands:

```console
yarn global add @picovoice/porcupine-node-demo
```

(or)

```console
npm install -g @picovoice/porcupine-node-demo
```

## Usage

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{OUTPUT_WAV_PATH}` as the file path to save the audio data in `wav` format.

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{OUTPUT_WAV_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
pvrecorder-node-demo --show_audio_devices
```

To run the audio recorder:

```console
pvrecorder-node-demo --audio_device_index {AUDIO_DEVICE_INDEX} --output_wav_path {OUTPUT_WAV_PATH}
```
