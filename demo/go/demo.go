// Copyright 2021-2023 Picovoice Inc.
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
	"flag"
	"log"
	"os"
	"os/signal"
	"path/filepath"

	pvrecorder "github.com/Picovoice/pvrecorder/binding/go"
	"github.com/go-audio/wav"
)

func main() {
	showAudioDevices := flag.Bool("show_audio_devices", false, "Display all the available input devices")
	audioDeviceIndex := flag.Int("audio_device_index", -1, "Index of audio input device to use.")
	outputWavPath := flag.String("output_wav_path", "", "Output path to save recorded a wav file.")
	flag.Parse()

	log.Printf("pvrecorder.go version: %s\n", pvrecorder.Version)

	if *showAudioDevices {
		log.Println("Printing devices...")
		if devices, err := pvrecorder.GetAvailableDevices(); err != nil {
			log.Fatalf("Error: %s.\n", err.Error())
		} else {
			for i, device := range devices {
				log.Printf("index: %d, device name: %s\n", i, device)
			}
		}
		return
	}

	recorder := pvrecorder.PvRecorder{
		DeviceIndex:         *audioDeviceIndex,
		FrameLength:         512,
		BufferedFramesCount: 10,
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

	go func() {
		<-signalCh
		close(waitCh)
	}()

	var outputWav *wav.Encoder
	if *outputWavPath != "" {
		outputFilePath, _ := filepath.Abs(*outputWavPath)
		outputFile, err := os.Create(outputFilePath)
		if err != nil {
			log.Fatalf("Failed to create output audio at path %s", outputFilePath)
		}
		defer outputFile.Close()

		outputWav = wav.NewEncoder(outputFile, pvrecorder.SampleRate, 16, 1, 1)
		defer outputWav.Close()
	}

waitLoop:
	for {
		select {
		case <-waitCh:
			log.Println("Stopping...")
			break waitLoop
		default:
			pcm, err := recorder.Read()
			if err != nil {
				log.Fatalf("Error: %s.\n", err.Error())
			}
			if outputWav != nil {
				for outputBufIndex := range pcm {
					err := outputWav.WriteFrame(pcm[outputBufIndex])
					if err != nil {
						log.Fatal(err)
					}
				}
			}
		}
	}
}
