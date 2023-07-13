# PvRecorder Binding for .NET

# PvRecorder

PvRecorder is an easy-to-use, cross-platform audio recorder designed for real-time speech audio processing. It allows developers access to an audio device's input stream, broken up into data frames of a given size.

## Requirements

- .NET 6.0

## Compatibility

Platform compatible with .NET Framework 4.6.1+:

- Windows (x86_64)

Platforms compatible with .NET Core 2.0+:

- Linux (x86_64)
- macOS (x86_64)
- Windows (x86_64)

Platforms compatible with .NET Core 3.0+:

- Raspberry Pi:
  - 2
  - 3 (32 and 64 bit)
  - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

Platform compatible with .NET 6.0+:

- macOS (arm64)

## Installation

You can install the latest version of PvRecorder by adding the latest [PvRecorder Nuget package](https://www.nuget.org/packages/PvRecorder/)
in Visual Studio or using by using the .NET CLI:

```console
dotnet add package PvRecorder
```

## Usage

Initialize and begin recording:

```csharp
using Pv;

PvRecorder recorder = PvRecorder.Create(frameLength: 512);
recorder.Start();
```

Read frames of audio:

```csharp
while (recorder.IsRecording)
{
    short[] frame = recorder.Read();
    // process audio frame
}
```

To stop recording:

```csharp
recorder.Stop();
```

Once you are done, free the resources acquired by PvRecorder. You do not have to call `Stop()` before `Dispose()`:

```csharp
recorder.Dispose();
```

To have resources freed immediately after use without explicitly calling the `Dispose()` function, wrap `PvRecorder` in a `using` statement:

```csharp
using (PvRecorder recorder = PvRecorder.Create(frameLength: 512)) {
    // PvRecorder usage
}
```

### Selecting an Audio Device

To print a list of available audio devices:
```csharp
string[] devices = PvRecorder.GetAudioDevices();
```

The index of the device in the returned list can be used in `Create()` to select that device for audio capture:
```csharp
PvRecorder recorder = PvRecorder.Create(
    frameLength: 512,
    deviceIndex: 2);
```


## Demo

The [PvRecorder .NET demo](https://github.com/Picovoice/pvrecorder/tree/main/demo/dotnet) is a .NET command-line application that demonstrates how to
use PvRecorder to record audio to a file.
