# PvRecorder Rust Binding

A cross platform audio recorder that captures single-channel audio at a sample rate of 16kHz.

## Requirements

- Rust 1.54+

## Compatibility

- Windows (x86_64)
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

To add the pvrecorder library into your app, add `pv_recorder` to your app's `Cargo.toml` manifest:
```toml
[dependencies]
pv_recorder = "*"
```

## Usage

Getting the list of input devices does not require an instance:

```rust
use pv_recorder::Recorder

let audio_devices = RecorderBuilder::default().get_audio_devices();
```

To start recording initialize an instance using the builder and run `start`:

```rust
use pv_recorder::RecorderBuilder;

let recorder = RecorderBuilder::new().init()?;
recorder.start()?
```

Get the pcm frames by calling the read function:

```rust
loop {
    let mut pcm_frame_buffer: [i16; FRAME_LENGTH] = [0; FRAME_LENGTH];
    recorder.read(&mut pcm_frame_buffer)?;
    // do something with pcm
}
```

To stop recording just run stop on the instance:

```rust
recorder.stop()?;
```

### Demo

For more detailed information on how to use the pv_recorder Rust sdk, see [examples/demo.rs](examples/demo.rs).

In the following instructions, we will refer to  `{AUDIO_DEVICE_INDEX}` as the index of the audio device to use, and `{OUTPUT_PATH}` as the path to save the raw audio data

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{OUTPUT_PATH}` can be empty if you wish to not save any data.

To show the available audio devices run:

```console
cargo run --release --example demo -- --show_audio_devices
```

To run the audio recorder:

```console
cargo run --release --example demo -- --audio_device_index {AUDIO_DEVICE_INDEX} --output_path {OUTPUT_PATH}
```

See additional options by calling `-h/--help`:

```console
cargo run --release --example demo -- -h
```
