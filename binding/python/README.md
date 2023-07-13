# PvRecorder Binding for Python

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Compatibility

- Python 3.5+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

pip3 install pvrecorder

## Usage

Initialize and begin recording:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(frame_length=512)
recorder.start()
```

(or)

Use `get_available_devices()` to get a list of available devices and then initialize the instance based on the index of a device:

```python
from pvrecorder import PvRecorder

devices = PvRecorder.get_available_devices()

recorder = PvRecorder(frame_length=512, device_index=0)
recorder.start()
```

Read frames of audio:

```python
while recorder.is_recording:
    frame = recorder.read()
    # process audio frame
```

To stop recording, call `stop()` on the instance:

```python
recorder.stop()
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `stop()` before `delete()`:

```python
recorder.delete()
```

## Demos

[pvrecorderdemo](https://pypi.org/project/pvrecorderdemo/) provides command-line utilities for recording audio to a file.
