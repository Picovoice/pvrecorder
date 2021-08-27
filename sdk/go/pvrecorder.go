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

int16_t *malloc_cgo(int32_t length) {
    int16_t *pcm = malloc(sizeof(int16_t) * length);
    return pcm;
}
*/
import "C"
import (
    "embed"
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

// PVRecorderStatus type
type PVRecorderStatus int

// PVRecorder status return codes from C library
const (
    SUCCESS                     PVRecorderStatus = 0
    OUT_OF_MEMORY               PVRecorderStatus = 1
    INVALID_ARGUMENT            PVRecorderStatus = 2
    INVALID_STATE               PVRecorderStatus = 3
    BACKEND_ERROR               PVRecorderStatus = 4
    DEVICE_ALREADY_INITIALIZED  PVRecorderStatus = 5
    DEVICE_NOT_INITIALIZED      PVRecorderStatus = 6
    IO_ERROR                    PVRecorderStatus = 7
    RUNTIME_ERROR               PVRecorderStatus = 8
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
    case IO_ERROR:
        return "IO_ERROR"
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

    // Index of audio device to start recording and capture audio.
    DeviceIndex int

    // FrameLength to get for each read command.
    FrameLength int

    // BufferSizeMSec is the total amount of audio frames to store in milliseconds.
    BufferSizeMSec int

    // LogOverflow flag to enable logs.
    LogOverflow int
}

type nativePVRecorderInterface interface {
    nativeInit(*PVRecorder)
    nativeDelete(*PVRecorder)
    nativeStart(*PVRecorder)
    nativeStop(*PVRecorder)
    nativeGetSelectedDevice(*PVRecorder)
    nativeGetAudioDevices(*C.int32_t, ***C.char)
    nativeFreeDeviceList(C.int32_t, **C.char)
    nativeVersion()
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

// Read function reads audio frames.
func (pvrecorder *PVRecorder) Read() ([]int16, error) {
    pcm := C.malloc_cgo(C.int32_t(pvrecorder.FrameLength))
    defer C.free(unsafe.Pointer(pcm))

    ret := nativePVRecorder.nativeRead(pvrecorder, pcm)
    if ret != SUCCESS {
        return nil, fmt.Errorf("PVRecorder Read failed with: %s", pvRecorderStatusToString(ret))
    }

    pcmCSlice := (*[1 << 28]C.int16_t)(unsafe.Pointer(pcm))[:pvrecorder.FrameLength:pvrecorder.FrameLength]
    pcmSlice := make([]int16, pvrecorder.FrameLength)
    for i := range pcmSlice {
        pcmSlice[i] = int16(pcmCSlice[i])
    }

    return pcmSlice, nil
}

// GetSelectedDevice gets the current selected audio input device name
func (pvrecorder *PVRecorder) GetSelectedDevice() string {
    return nativePVRecorder.nativeGetSelectedDevice(pvrecorder)
}

// GetAudioDevices function gets the currently available input audio devices.
func GetAudioDevices() ([]string, error) {
    var count int
    var devices **C.char

    if ret := nativePVRecorder.nativeGetAudioDevices(&count, &devices); ret != SUCCESS {
        return nil, fmt.Errorf("PVRecorder GetAudioDevices failed with: %s", pvRecorderStatusToString(ret))
    }
    defer nativePVRecorder.nativeFreeDeviceList(count, devices)

    deviceSlice := (*[1 << 28]*C.char)(unsafe.Pointer(devices))[:count:count]

    deviceNames := make([]string, count)
    for i := 0; i < count; i++ {
        deviceNames[i] = C.GoString(deviceSlice[i])
    }

    return deviceNames, nil
}

// Version function gets the current library version.
func Version() string {
    return nativePVRecorder.nativeVersion()
}

func extractLib() string {
    var scriptPath string
    if runtime.GOOS == "windows" {
        scriptPath = "embedded/scripts/platform.bat"
    } else {
        scriptPath = "embedded/scripts/platform.sh"
    }

    script := extractFile(scriptPath, extractionDir)
    cmd := exec.Command(script)
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

    srcPath := fmt.Sprintf("embedded/lib/%s/%s/%s", osName, cpu, libFile)

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
