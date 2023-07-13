# PvRecorder Demo for Python

This project contains a Python command-line demo for PvRecorder that demonstrates how to use PvRecorder to record audio to a WAV file.

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Compatibility

- Python 3.5+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

```console
pip3 install pvrecorderdemo
```

## Usage

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{OUTPUT_WAV_PATH}` as the path to save the audio data in `wav` format.

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{OUTPUT_WAV_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
pv_recorder_demo --show_audio_devices
```

To run PvRecorder run:

```console
pv_recorder_demo --audio_device_index {AUDIO_DEVICE_INDEX} --output_wav_path {OUTPUT_WAV_PATH}
```
