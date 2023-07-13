// Copyright 2023 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is
// located in the "LICENSE" file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing permissions and
// limitations under the License.

package pvrecorder

import (
	"os"
	"testing"
)

func TestMain(m *testing.M) {
	os.Exit(m.Run())
}

func TestInvalidDeviceIndex(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.DeviceIndex = -2

	err := recorder.Init()
	if err == nil {
		t.Fatalf("Init succeeded with invalid device index")
	} else {
		recorder.Delete()
	}
}

func TestInvalidFrameLength(t *testing.T) {
	recorder := NewPvRecorder(0)

	err := recorder.Init()
	if err == nil {
		t.Fatalf("Init succeeded with invalid frame length")
	} else {
		recorder.Delete()
	}
}

func TestInvalidBufferedFramesCount(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.BufferedFramesCount = 0

	err := recorder.Init()
	if err == nil {
		t.Fatalf("Init succeeded with invalid buffered frames count")
	} else {
		recorder.Delete()
	}
}

func TestStartStop(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.DeviceIndex = 0

	err := recorder.Init()
	if err != nil {
		t.Fatalf("Init failed: %v", err)
	}
	defer recorder.Delete()

	err = recorder.Start()
	if err != nil {
		t.Fatalf("Start failed: %v", err)
	}

	for i := 0; i < 5; i++ {
		frame, err := recorder.Read()
		if err != nil {
			t.Fatalf("Read failed: %v", err)
		}
		if len(frame) != recorder.FrameLength {
			t.Fatalf("Frame length is not equal")
		}
	}
}

func TestSetDebugLogging(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.DeviceIndex = 0

	err := recorder.Init()
	if err != nil {
		t.Fatalf("Init failed: %v", err)
	}
	defer recorder.Delete()

	recorder.SetDebugLogging(true)
	recorder.SetDebugLogging(false)
}

func TestGetIsRecording(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.DeviceIndex = 0

	err := recorder.Init()
	if err != nil {
		t.Fatalf("Init failed: %v", err)
	}
	defer recorder.Delete()

	err = recorder.Start()
	if err != nil {
		t.Fatalf("Start failed: %v", err)
	}

	isRecording := recorder.IsRecording()
	if !isRecording {
		t.Fatalf("Recorder should be recording audio")
	}

	err = recorder.Stop()
	if err != nil {
		t.Fatalf("Stop failed: %v", err)
	}

	isRecording = recorder.IsRecording()
	if isRecording {
		t.Fatalf("Recorder should have stopped recording audio")
	}
}

func TestGetSelectedDevice(t *testing.T) {
	recorder := NewPvRecorder(512)
	recorder.DeviceIndex = 0

	err := recorder.Init()
	if err != nil {
		t.Fatalf("Init failed: %v", err)
	}
	defer recorder.Delete()

	device := recorder.GetSelectedDevice()

	if len(device) == 0 {
		t.Fatalf("Failed to get selected device")
	}
}

func TestGetAvailableDevices(t *testing.T) {
	_, err := GetAvailableDevices()
	if err != nil {
		t.Fatalf("Failed to get available devices: %v", err)
	}
}

func TestVersion(t *testing.T) {
	version := Version
	if len(version) == 0 {
		t.Fatalf("Failed to get version")
	}
}

func TestSampleRate(t *testing.T) {
	sampleRate := SampleRate
	if sampleRate <= 0 {
		t.Fatalf("Failed to get sample rate")
	}
}
