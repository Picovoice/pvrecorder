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

package pvrecorder

/*
#include <stdint.h>
#include <stdlib.h>
*/
import "C"
import (
	"embed"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"reflect"
	"runtime"
	"strings"
	"unsafe"
)

//go:embed embedded
var embeddedFS embed.FS

// PVRecorderStatus type
type PVRecorderStatus int

// PVRecorder status return codes from C library
const (
	SUCCESS						PVRecorderStatus = 0
	OUT_OF_MEMORY 				PVRecorderStatus = 1
	INVALID_ARGUMENT			PVRecorderStatus = 2
	INVALID_STATE				PVRecorderStatus = 3
	BACKEND_ERROR				PVRecorderStatus = 4
	DEVICE_ALREADY_INITIALIZED	PVRecorderStatus = 5
	DEVICE_NOT_INITIALIZED 		PVRecorderStatus = 6
	RUNTIME_ERROR 				PVRecorderStatus = 7
)

func pvRecorderStatusToString(status PVRecorderStatus) string {
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
	case RUNTIME_ERROR:
		return "RUNTIME_ERROR"
	default:
		return fmt.Sprintf("Unknown error code: %d", status)
	}
}

// PVRecorder struct
type PVRecorder struct {
	// handle for pvrecorder instance in C.
	handle uintptr

	// Extra userData to be used in the callback.
	userData *userDataType

	// Index of audio device to start recording.
	DeviceIndex int

	// FrameLength of the audio buffer. Callback uses this amount of frames each iteration.
	FrameLength int

	// Callback to process the audio frames each iteration.
	Callback func ([]int16)
}

type userDataType struct {
	callback func([]int16)
	frameLength int
}

type nativePVRecorderInterface interface {
	nativeInit(*PVRecorder)
	nativeStart(*PVRecorder)
	nativeStop(*PVRecorder)
	nativeDelete(*PVRecorder)
}

type nativePVRecorderType struct {}

// private vars
var (
	extractionDir = filepath.Join(os.TempDir(), "pvrecorder")
	libName = extractLib()
	nativePVRecorder = nativePVRecorderType{}
)

// Init function for PVRecorder
func (pvrecorder *PVRecorder) Init() error {
	pvrecorder.userData = &userDataType{
		callback: pvrecorder.Callback,
		frameLength: pvrecorder.FrameLength,
	}

	ret := nativePVRecorder.nativeInit(pvrecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PVRecorder Init failed with: %s", pvRecorderStatusToString(ret))
	}

	return nil
}

// Delete function releases resources acquired by PVRecorder
func (pvrecorder *PVRecorder) Delete() {
	nativePVRecorder.nativeDelete(pvrecorder)
}

// Start function starts recording audio.
func (pvrecorder *PVRecorder) Start() error {
	ret := nativePVRecorder.nativeStart(pvrecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PVRecorder Start failed with: %s", pvRecorderStatusToString(ret))
	}

	return nil
}

// Stop function stops recording audio.
func (pvrecorder *PVRecorder) Stop() error {
	ret := nativePVRecorder.nativeStop(pvrecorder)
	if ret != SUCCESS {
		return fmt.Errorf("PVRecorder Stop failed with: %s", pvRecorderStatusToString(ret))
	}

	return nil
}

// GetAudioDevices function gets the currently available input audio devices.
func GetAudioDevices() ([]string, error) {
	var count int
	var devices **C.char

	if ret := nativeGetAudioDevices(&count, &devices); ret != SUCCESS {
		return nil, fmt.Errorf("PVRecorder GetAudioDevices failed with: %s", pvRecorderStatusToString(ret))
	}
	defer nativeFreeDeviceList(count, devices)

	var deviceSlice []*C.char
	sh := (*reflect.SliceHeader)(unsafe.Pointer(&deviceSlice))
	sh.Data = uintptr(unsafe.Pointer(devices))
	sh.Cap = count
	sh.Len = count

	deviceNames := make([]string, count)
	for i := 0; i < count; i++ {
		deviceNames[i] = C.GoString(deviceSlice[i])
	}

	return deviceNames, nil
}

//export callbackHandler
func callbackHandler(pcm *C.int16_t, userData unsafe.Pointer) {
	if userData != nil {
		req := (*userDataType)(userData)

		var pcmSlice []int16

		sh := (*reflect.SliceHeader)(unsafe.Pointer(&pcmSlice))
		sh.Data = uintptr(unsafe.Pointer(pcm))
		sh.Len = req.frameLength
		sh.Cap = req.frameLength

		req.callback(pcmSlice)
	}
}

func extractLib() string {
	var command string
	var scriptPath string
	if runtime.GOOS == "windows" {
		command = "cmd"
		scriptPath = path.Join("embedded", "scripts", "platform.bat")
	} else {
		command = "bash"
		scriptPath = path.Join("embedded", "scripts", "platform.sh")
	}

	cmd := exec.Command(command, scriptPath)
	stdout, err := cmd.Output()
	
	if err != nil {
		log.Fatal("System is not supported.")
	}

	systemInfo := strings.Split(string(stdout), " ")
	osName := systemInfo[0]
	cpu := systemInfo[1]

	libFile := "libpv_recorder"
	if runtime.GOOS == "windows" {
		libFile += ".dll"
	} else if runtime.GOOS == "darwin" {
		libFile += ".dylib"
	} else if runtime.GOOS == "linux" {
		libFile += ".so"
	} else {
		log.Fatalf("OS: %s is not supported.", runtime.GOOS)
	}

	srcPath := filepath.Join("embedded", "lib", osName, cpu, libFile)

	return extractFile(srcPath, extractionDir)
}

func extractFile(srcFile string, dstDir string) string {
	bytes, readErr := embeddedFS.ReadFile(srcFile)
	if readErr != nil {
		log.Fatalf("%v", readErr)
	}

	extractedFilepath := filepath.Join(dstDir, srcFile)
	os.MkdirAll(filepath.Dir(extractedFilepath), 0777)
	writeErr := ioutil.WriteFile(extractedFilepath, bytes, 0777)
	if writeErr != nil {
		log.Fatalf("%v", writeErr)
	}
	return extractedFilepath
}