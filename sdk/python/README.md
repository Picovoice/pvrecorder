# PV_Recorder

A cross platform audio recorder to read one channel and 16kHz samples.

## Requirements

- Python 3

## Compatibility

- Windows (amd64)
- macOS (x86_64)
- Linux (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Installation

pip3 install pvrecorder

## Usage

Getting the list of input devices does not require an instance:

```python
from pvporcupine import PVRecorder

devices = PVRecorder.get_audio_devices()
```

Before recording, create a callback that processes the pcm:

```python
def callback(pcm):
    # do something with pcm
    print(len(pcm))
```

To start recording initialize the instance and run start:

```python
from pvporcupine import PVRecorder

recorder = PVRecorder(device_index=-1, frame_length=512, callback=callback)
recorder.start()
```

To stop recording just run stop on the instance:

```python
recorder.stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```python
recorder.delete()
```