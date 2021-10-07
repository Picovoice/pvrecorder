# PV_Recorder

A cross platform audio recorder to read one channel and 16kHz samples.

## Requirements

- Python 3

## Compatibility

- Windows (amd64)
- macOS 
    - x86_64
    - arm64
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
from pvrecorder import PVRecorder

devices = PVRecorder.get_audio_devices()
```

To start recording initialize the instance and run start:

```python
from pvrecorder import PvRecorder

recorder = PvRecorder(device_index=-1, frame_length=512)
recorder.start()
```

Get the pcm frames by calling the read function:

```python
pcm = recorder.read()
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

### Demo

For more detailed information on how to use the pv_recorder python sdk, please that a look at [demo.py](demo/demo.js).

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{OUTPUT_PATH}` as the path to save the audio data in `wav` format.

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{OUTPUT_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
python3 demo.py --show_audio_devices
```

To run audio recorder run:

```console
python3 demo.py --audio_device_index {AUDIO_DEVICE_INDEX} --output_path {OUTPUT_PATH}
```
