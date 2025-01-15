// Copyright 2021-2025 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is
// located in the "LICENSE" file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing permissions and
// limitations under the License.
//

package pvrecorder

/*
#include <stdint.h>
#include <stdlib.h>

int16_t *malloc_cgo(int32_t length) {
    int16_t *pcm = malloc(sizeof(int16_t) * length);
    return pcm;
}
*/
import "C"
import (
	"crypto/sha256"
	"embed"
	"encoding/hex"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"unsafe"
)

//go:embed embedded
var embeddedFS embed.FS

// PvRecorderStatus type
type PvRecorderStatus int

// PvRecorder status return codes from C library
const (
	SUCCESS                    PvRecorderStatus = 0
	OUT_OF_MEMORY              PvRecorderStatus = 1
	INVALID_ARGUMENT           PvRecorderStatus = 2
	INVALID_STATE              PvRecorderStatus = 3
	BACKEND_ERROR              PvRecorderStatus = 4
	DEVICE_ALREADY_INITIALIZED PvRecorderStatus = 5
	DEVICE_NOT_INITIALIZED     PvRecorderStatus = 6
	IO_ERROR                   PvRecorderStatus = 7
	RUNTIME_ERROR              PvRecorderStatus = 8
)

func pvRecorderStatusToString(status PvRecorderStatus) string {
	switch status {
	case SUCCESS:
		return "SUCCESS"
	case OUT_OF_MEMORY:
		return "OUT_OF_MEMORY"
	case INVALID_ARGUMENT:
		return "INVALID_ARGUMENT"
	case INVALID_STATE:
		return "INVALID_STATE"
	case BACKEND_ERROR:
		return "BACKEND_ERROR"
	case DEVICE_ALREADY_INITIALIZED:
		return "DEVICE_ALREADY_INITIALIZED"
	case DEVICE_NOT_INITIALIZED:
		return "DEVICE_NOT_INITIALIZED"
	case IO_ERROR:
		return "IO_ERROR"
	case RUNTIME_ERROR:
		return "RUNTIME_ERROR"
	default:
		return fmt.Sprintf("Unknown error code: %d", status)
	}
}

// PvRecorder struct
type PvRecorder struct {
	// handle for pvRecorder instance in C.
	handle uintptr

	// Length of each frame returned from read command.
	FrameLength int

	// Index of audio device to start recording and capture audio.
	DeviceIndex int

	// BufferedFramesCount is the number of audio frames buffered internally for reading - i.e. internal circular buffer
	// will be of size `frame_length` * `buffered_frames_count`. If this value is too low, buffer overflows could occur.
	BufferedFramesCount int
}

type nativePvRecorderInterface interface {
	nativeInit(*PvRecorder)
	nativeDelete(*PvRecorder)
	nativeStart(*PvRecorder)
	nativeStop(*PvRecorder)
	nativeRead(*PvRecorder)
	nativeSetDebugLogging(*PvRecorder)
	nativeGetSelectedDevice(*PvRecorder)
	nativeGetAudioDevices(*C.int32_t, ***C.char)
	nativeFreeDeviceList(C.int32_t, **C.char)
	nativeVersion()
	nativeSampleRate()
}

type nativePvRecorderType struct{}

// private vars
var (
	osName, cpu      = getOS()
	extractionDir    = filepath.Join(os.TempDir(), "cheetah")

	libName          = extractLib()
	nativePvRecorder = nativePvRecorderType{}
)

var (
	// SampleRate Audio sample rate used by PvRecorder.
	SampleRate int

	// Version PvRecorder version
	Version string
)

// NewPvRecorder returns a PvRecorder struct with default parameters
func NewPvRecorder(frameLength int) PvRecorder {
	return PvRecorder{
		FrameLength:         frameLength,
		DeviceIndex:         -1,
		BufferedFramesCount: 50,
	}
}

// Init function for PvRecorder
func (pvRecorder *PvRecorder) Init() error {
	ret := nativePvRecorder.nativeInit(pvRecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PvRecorder Init failed with: %s", pvRecorderStatusToString(ret))
	}

	SampleRate = nativePvRecorder.nativeSampleRate()
	Version = nativePvRecorder.nativeVersion()

	return nil
}

// Delete function releases resources acquired by PvRecorder
func (pvRecorder *PvRecorder) Delete() {
	nativePvRecorder.nativeDelete(pvRecorder)
}

// Start function starts recording audio.
func (pvRecorder *PvRecorder) Start() error {
	ret := nativePvRecorder.nativeStart(pvRecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PvRecorder Start failed with: %s", pvRecorderStatusToString(ret))
	}

	return nil
}

// Stop function stops recording audio.
func (pvRecorder *PvRecorder) Stop() error {
	ret := nativePvRecorder.nativeStop(pvRecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PvRecorder Stop failed with: %s", pvRecorderStatusToString(ret))
	}

	return nil
}

// Read function reads audio frames.
func (pvRecorder *PvRecorder) Read() ([]int16, error) {
	pcm := C.malloc_cgo(C.int32_t(pvRecorder.FrameLength))
	defer C.free(unsafe.Pointer(pcm))

	ret := nativePvRecorder.nativeRead(pvRecorder, pcm)
	if ret != SUCCESS {
		return nil, fmt.Errorf("PvRecorder Read failed with: %s", pvRecorderStatusToString(ret))
	}

	pcmCSlice := (*[1 << 28]C.int16_t)(unsafe.Pointer(pcm))[:pvRecorder.FrameLength:pvRecorder.FrameLength]
	pcmSlice := make([]int16, pvRecorder.FrameLength)
	for i := range pcmSlice {
		pcmSlice[i] = int16(pcmCSlice[i])
	}

	return pcmSlice, nil
}

// SetDebugLogging enables or disables debug logging for PvRecorder. Debug logs will indicate when there are
// overflows in the internal  frame buffer and when an audio source is generating frames of silence.
func (pvRecorder *PvRecorder) SetDebugLogging(isDebugLoggingEnabled bool) {
	nativePvRecorder.nativeSetDebugLogging(pvRecorder, isDebugLoggingEnabled)
}

// IsRecording gets whether the given instance is currently recording audio or not.
func (pvRecorder *PvRecorder) IsRecording() bool {
	return nativePvRecorder.nativeGetIsRecording(pvRecorder)
}

// GetSelectedDevice gets the current selected audio input device name
func (pvRecorder *PvRecorder) GetSelectedDevice() string {
	return nativePvRecorder.nativeGetSelectedDevice(pvRecorder)
}

// GetAvailableDevices function gets the currently available input audio devices.
func GetAvailableDevices() ([]string, error) {
	var count int
	var devices **C.char

	if ret := nativePvRecorder.nativeGetAudioDevices(&count, &devices); ret != SUCCESS {
		return nil, fmt.Errorf("PvRecorder GetAudioDevices failed with: %s", pvRecorderStatusToString(ret))
	}
	defer nativePvRecorder.nativeFreeDeviceList(count, devices)

	deviceSlice := (*[1 << 28]*C.char)(unsafe.Pointer(devices))[:count:count]

	deviceNames := make([]string, count)
	for i := 0; i < count; i++ {
		deviceNames[i] = C.GoString(deviceSlice[i])
	}

	return deviceNames, nil
}

func getOS() (string, string) {
	switch os := runtime.GOOS; os {
	case "darwin":
		return "mac", getMacArch()
	case "linux":
		osName, cpu := getLinuxDetails()
		return osName, cpu
	case "windows":
		return "windows", getWinArch()
	default:
		log.Fatalf("%s is not a supported OS", os)
		return "", ""
	}
}

func getMacArch() string {
	if runtime.GOARCH == "arm64" {
		return "arm64"
	} else {
		return "x86_64"
	}
}

func getWinArch() string {
	if runtime.GOARCH == "arm64" {
		return "arm64"
	} else {
		return "amd64"
	}
}

func getLinuxDetails() (string, string) {
	var archInfo = ""

	if runtime.GOARCH == "amd64" {
		return "linux", "x86_64"
	} else if runtime.GOARCH == "arm64" {
		archInfo = "-aarch64"
	}

	cmd := exec.Command("cat", "/proc/cpuinfo")
	cpuInfo, err := cmd.Output()

	if err != nil {
		log.Fatalf("Failed to get CPU details: %s", err.Error())
	}

	var cpuPart = ""
	for _, line := range strings.Split(string(cpuInfo), "\n") {
		if strings.Contains(line, "CPU part") {
			split := strings.Split(line, " ")
			cpuPart = strings.ToLower(split[len(split)-1])
			break
		}
	}

	switch cpuPart {
	case "0xb76":
		return "raspberry-pi", "arm11" + archInfo
	case "0xd03":
		return "raspberry-pi", "cortex-a53" + archInfo
	case "0xd08":
		return "raspberry-pi", "cortex-a72" + archInfo
	case "0xd0b":
		return "raspberry-pi", "cortex-a76" + archInfo
	default:
		log.Fatalf("Unsupported CPU:\n%s", cpuPart)
		return "", ""
	}
}

func extractLib() string {
	var libPath string
	switch os := runtime.GOOS; os {
	case "darwin":
		libPath = fmt.Sprintf("embedded/lib/%s/%s/libpv_cheetah.dylib", osName, cpu)
	case "linux":
		if cpu == "" {
			libPath = fmt.Sprintf("embedded/lib/%s/libpv_cheetah.so", osName)
		} else {
			libPath = fmt.Sprintf("embedded/lib/%s/%s/libpv_cheetah.so", osName, cpu)
		}
	case "windows":
		libPath = fmt.Sprintf("embedded/lib/%s/amd64/libpv_cheetah.dll", osName)
	default:
		log.Fatalf("%s is not a supported OS", os)
	}

	return extractFile(libPath, extractionDir)
}

func extractFile(srcFile string, dstDir string) string {
	bytes, readErr := embeddedFS.ReadFile(srcFile)
	if readErr != nil {
		log.Fatalf("%v", readErr)
	}

	extractedFilepath := filepath.Join(dstDir, srcFile)
	err := os.MkdirAll(filepath.Dir(extractedFilepath), 0777)
	if err != nil {
		log.Fatalf("%v", err)
	}

	writeErr := ioutil.WriteFile(extractedFilepath, bytes, 0777)
	if writeErr != nil {
		log.Fatalf("%v", writeErr)
	}
	return extractedFilepath
}

func sha256sum(filePath string) string {
	bytes, readErr := embeddedFS.ReadFile(filePath)
	if readErr != nil {
		log.Fatalf("%v", readErr)
	}

	sum := sha256.Sum256(bytes)
	return hex.EncodeToString(sum[:])
}
