// Copyright 2021 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is
// located in the "LICENSE" file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing permissions and
// limitations under the License.
//

package main

import (
    "bytes"
    "encoding/binary"
    "flag"
    "log"
    "os"
    "os/signal"

    pvrecorder "github.com/Picovoice/pvrecorder/sdk/go"
)

func main() {
    showAudioDevices := flag.Bool("show_audio_devices", false, "Display all the available input devices")
    audioDeviceIndex := flag.Int("audio_device_index", -1, "Index of audio input device to use.")
    rawOutputPath := flag.String("raw_output_path", "", "Output path to save recorded audio raw bytes.")
    flag.Parse()

    log.Printf("pvrecorder.go version: %s\n", pvrecorder.Version())

    if *showAudioDevices {
        log.Println("Printing devices...")
        if devices, err := pvrecorder.GetAudioDevices(); err != nil {
            log.Fatalf("Error: %s.\n", err.Error())
        } else {
            for i, device := range devices {
                log.Printf("index: %d, device name: %s\n", i, device)
            }
        }
        return;
    }

    recorder := pvrecorder.PvRecorder{
        DeviceIndex: *audioDeviceIndex,
        FrameLength: 512,
        BufferSizeMSec: 1000,
        LogOverflow: 1,
    }

    log.Println("Initializing...")
    if err := recorder.Init(); err != nil {
        log.Fatalf("Error: %s.\n", err.Error())
    }
    defer recorder.Delete()

    log.Printf("Using device: %s", recorder.GetSelectedDevice())

    log.Println("Starting...")
    if err := recorder.Start(); err != nil {
        log.Fatalf("Error: %s.\n", err.Error())
    }

    signalCh := make(chan os.Signal, 1)
    waitCh := make(chan struct{})
    signal.Notify(signalCh, os.Interrupt)

    go func () {
        <- signalCh
        close(waitCh)
    }()

    var f *os.File
    var buf *bytes.Buffer
    if *rawOutputPath != "" {
        file, err := os.Create(*rawOutputPath)
        if err != nil {
            log.Fatalf("Failed to create file: %s\n", err.Error())
        }
        defer file.Close()

        f = file
        buf = new(bytes.Buffer)
    }
    
    waitLoop:
    for {
        select {
        case <- waitCh:
            log.Println("Stopping...")
            break waitLoop
        default:
            pcm, err := recorder.Read()
            if err != nil {
                log.Fatalf("Error: %s.\n", err.Error())
            }
            if *rawOutputPath != "" {
                if err := binary.Write(buf, binary.LittleEndian, pcm); err != nil {
                    log.Fatalf("Failed to write pcm to buffer: %s.\n", err.Error())
                }
                if _, err := f.Write(buf.Bytes()); err != nil {
                    log.Fatalf("Failed to write bytes to file: %s\n", err.Error())
                }
                buf.Reset()
            }
        }
    }
}