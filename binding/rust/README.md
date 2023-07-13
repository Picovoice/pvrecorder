# PvRecorder Binding for Python

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Requirements

- Rust 1.54+

## Compatibility

- Linux (x86_64)
- macOS (x86_64 and arm64)
- Windows (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Installation

To add the pvrecorder library into your app, add `pv_recorder` to your app's `Cargo.toml` manifest:
```toml
[dependencies]
pv_recorder = "*"
```

## Usage

Getting the list of input devices does not require an instance:

```rust
use pv_recorder::PvRecorderBuilder

let audio_devices = PvRecorderBuilder::default().get_audio_devices()?;
```

To start recording, initialize an instance using the builder and call `start()`:

```rust
use pv_recorder::PvRecorderBuilder;

let frame_length = 512;
let recorder = PvRecorderBuilder::new(frame_length).init()?;
recorder.start()?
```

Read frames of audio:

```rust
while recorder.is_recording() {
    let frame = recorder.read()?;
    // process audio frame
}
```

To stop recording, call `stop()` on the instance:

```rust
recorder.stop()?;
```

## Demo

The [PvRecorder Rust demo](https://github.com/Picovoice/pvrecorder/tree/main/demo/rust) is a Rust command-line application that demonstrates how to
use PvRecorder to record audio to a file.
