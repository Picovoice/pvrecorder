# PvRecorder Demo for .NET

This project contains a .NET command-line demo for PvRecorder that demonstrates how to use PvRecorder to record audio to a WAV file.

## PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Requirements

- .NET 6.0

## Compatibility

- Linux (x86_64)
- macOS (x86_64, arm64)
- Windows (x86_64)
- Raspberry Pi:
  - 2
  - 3 (32 and 64 bit)
  - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Installation

This demo uses [Microsoft's .NET SDK](https://dotnet.microsoft.com/download).

Build with the dotnet CLI:

```console
dotnet build
```

## Usage

To build the demo:

```console
cd demo/dotnet/PvRecorderDemo
dotnet build
```

To show the available audio devices run:

```console
dotnet run -- --show_audio_devices
```

To run the demo, give it a file to record audio to:

```console
dotnet run -- --output_wav_path ${OUTPUT_WAV_FILE}
```

You can also select the audio device index to use for recording (use `--show_audio_devices` to see options):

```console
dotnet run -- --output_wav_path ${OUTPUT_WAV_FILE} --audio_device_index ${DEVICE_INDEX}
```