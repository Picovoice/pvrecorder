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
	"log"
	"time"

	pvrecorder "github.com/Picovoice/pvrecorder/sdk/go"
)

func callback(pcm []int16) {
	log.Printf("Received pcm with length: %d\n", len(pcm))
	// do something with pcm
}

func main() {
	log.Println("Printing devices...")
	if devices, err := pvrecorder.GetAudioDevices(); err != nil {
		log.Fatalf("Error: %s.\n", err.Error())
	} else {
		for i, device := range devices {
			log.Printf("index: %d, device name: %s\n", i, device)
		}
	}
	log.Println("")

	recorder := pvrecorder.PVRecorder{
		DeviceIndex: -1,
		FrameLength: 512,
		Callback: callback,
	}

	log.Println("Initializing...")
	if err := recorder.Init(); err != nil {
		log.Fatalf("Error: %s.\n", err.Error())
	}

	log.Println("Starting...")
	if err := recorder.Start(); err != nil {
		log.Fatalf("Error: %s.\n", err.Error())
	}

	log.Println("Wating for 3 seconds...")
	time.Sleep(3 * time.Second)

	log.Println("Deleting...")
	recorder.Delete()
}