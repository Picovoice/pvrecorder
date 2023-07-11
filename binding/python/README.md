# PvRecorder Binding for Python

## Compatibility

- Python 3.5+
- Runs on Linux (x86_64), macOS (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

pip3 install pvrecorder

## Usage

To start recording initialize an instance and run start:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(frame_length=512)
recorder.start()
```

(or)

Use `get_available_devices()` to get a list of available devices and then initialize the instance based on the index of a device:

```python
from pvrecorder import PvRecorder

devices = PvRecorder.get_available_devices() # select index of device

recorder = PvRecorder(frame_length=512, device_index=0)
recorder.start()
```

Get a frame of audio by calling the `read()` function:

```python
frame = recorder.read()
# do something with frame
```

To stop recording just run stop on the instance:

```python
recorder.stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```python
recorder.delete()
```

## Demos

[pvrecorderdemo](https://pypi.org/project/pvrecorderdemo/) provides command-line utilities for recording audio to a file.
