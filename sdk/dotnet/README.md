# PV_Recorder

A cross platform audio recorder to read one channel and 16kHz samples.

## Requirements

- .NET Standard 2.0, .NET Core 3.1

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

```console
dotnet add package PvRecorder
```

## Usage

Getting the list of input devices does not require an instance:

```csharp
using Pv;

PvRecorder recorder = PvRecorder.Create(
    deviceIndex: -1, // uses default index
    frameLength: 512,
);
```

To start recording initialize the instance and run start:

```csharp
recorder.Start()
```

Get the pcm frames by calling the read function:

```csharp
while (true) {
    pcm = recorder.Read()
    // do something with pcm
}
```

To stop recording just run stop on the instance:

```csharp
recorder.Stop()
```

Once you are done, free the used resources. You do not have to call stop before Dispose:

```csharp
recorder.Dispose()
```

### Demo

For more detailed information on how to use the pv_recorder .NET sdk, please that a look at [Demo/Demo.cs](Demo/Demo.cs). 

In the following instructions we will refer to `{FRAMEWORK}` as the .NET framework to use
(`netcoreapp3.1` if .NET Core 3.1, `netstandard2.0` if .NET Standard 2.0), `{AUDIO_DEVICE_INDEX}`
as the audio device index to use, and `{RAW_OUTPUT_PATH}` as the path to save the raw audio data 

`{AUDIO_DEVICE_INDEX}` defaults to -1 and `{RAW_OUTPUT_PATH}` can be empty if you wish to not save any data.

To build the demo:

```console
dotnet build -c Release
```

To show the available audio devices run:

```console
dotnet run --project Demo -c Release --framework {FRAMEWORK} -- --show_audio_devices
```

To run audio recorder run:

```console
dotnet run --project Demo -c Release --framework {FRAMEWORK}
-- --audio_device_index {AUDIO_DEVICE_INDEX} --raw_output_path {RAW_OUTPUT_PATH}
```
