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

To start recording initialize the instance, choose an index from the previous command or set to `-1` to 
use default audio device, and set the max capacity of audio buffer:

```python
from pvporcupine import PVRecorder

recorder = PVRecorder(device_index=-1, capacity=2048)
recorder.start()
```

To constantly get the recorded audio run:\
**Note**: it is recommended to set the capacity when initializing to at least double the frame length reading each cycle.

```python
while True:
    pcm = recorder.read(512)
    # do something with pcm
```

To stop recording just run stop on the instance:

```python
recorder.stop()
```

Once you are done, free the used resources. You do not have to call stop before delete:

```python
recorder.delete()
```