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

import (
	"C"
	"log"
	"os/exec"
	"path"
	"runtime"
)
import "strings"

//export callback_wrapper
func callback_wrapper(callback func (*int16)) {
	
}

type PVRecorderStatus int

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

type PVRecorder struct {
	handle uintptr
	deviceIndex int32
	frameLength int32
	callback func (*int16)
}

type nativePVRecorderInterface interface {
	nativeInit(*PVRecorder)
	nativeStart(*PVRecorder)
	nativeStop(*PVRecorder)
	nativeDelete(*PVRecorder)
}

type nativePVRecorderType struct {}

func getLibPath() string {
	var scriptPath string
	if runtime.GOOS == "windows" {
		scriptPath = path.Join("scripts", "platform.bat")
	} else {
		scriptPath = path.Join("scripts", "platform.sh")
	}

	cmd := exec.Command(scriptPath)
	stdout, err := cmd.Output()
	
	if err != nil {
		log.Fatal("System is not supported.")
	}

	system_info := strings.Split(string(stdout), " ")
	os_name := system_info[0]
	cpu := system_info[1]

	lib_name := "libpv_recorder"
	if runtime.GOOS == "windows" {
		lib_name += ".dll"
	} else if runtime.GOOS == "darwin" {
		lib_name += ".dylib"
	} else if runtime.GOOS == "linux" {
		lib_name += ".so"
	} else {
		log.Fatalf("OS: %s is not supported.", runtime.GOOS)
	}

	return path.Join("lib", os_name, cpu, lib_name)
}
