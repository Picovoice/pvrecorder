# PvRecorder Demo for Rust

This project contains a Rust command-line demo for PvRecorder that demonstrates how to use PvRecorder to record audio to a WAV file.

## PvRecorder

PvRecorder is a user-friendly, cross-platform audio recorder that has been specifically designed for real-time audio processing.
It provides developers with access to the input stream of an audio device, which is divided into data frames of a given size.

## Requirements

- Rust 1.54+

## Compatibility

- Linux (x86_64)
- macOS (x86_64)
- Windows (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Usage

To build and run the demo:

```console
cargo run --release --
```

To show the available audio devices run:

```console
cargo run --release -- --show_audio_devices
```

To run the demo, give it a file to record audio to:

```console
cargo run --release -- --output_wav_path ${OUTPUT_WAV_FILE}
```

You can also select the audio device index to use for recording (use `--show_audio_devices` to see options):

```console
cargo run --release -- --output_wav_path ${OUTPUT_WAV_FILE} --audio_device_index ${DEVICE_INDEX}
```
