# PvRecorder

A cross-platform audio recorder to read one channel and 16kHz samples.

## Compatibility

- Python 3.5+
- Runs on Linux (x86_64), Mac (x86_64 and arm64), Windows (x86_64), Raspberry Pi (all variants), NVIDIA Jetson (Nano), and BeagleBone.

## Installation

pip3 install pvrecorder

## Usage

Getting the list of input devices does not require an instance:

```python
from pvrecorder import PvRecorder

devices = PvRecorder.get_audio_devices()
```

To start recording initialize an instance and run start:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(device_index=-1, frame_length=512)
recorder.start()
```

Get the frames by calling the read function:

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

[pvrecorderdemo](https://pypi.org/project/pvrecorderdemo/) provides command-line utilities for recording audio.
