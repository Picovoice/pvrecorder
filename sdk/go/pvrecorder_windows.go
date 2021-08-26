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

// +build windows

package pvrecorder

/*
#include <stdlib.h>
#include <stdint.h>

extern void callbackHandler(int16_t *pcm, void *userData);
*/
import "C"

import (
	"unsafe"

	"golang.org/x/sys/windows"
)

// private vars
var (
	lib = windows.NewLazyDLL(libName)

	pv_recorder_init_func 				= lib.NewProc("pv_recorder_init")
	pv_recorder_delete_func 			= lib.NewProc("pv_recorder_delete")
	pv_recorder_start_func 				= lib.NewProc("pv_recorder_start")
	pv_recorder_stop_func 				= lib.NewProc("pv_recorder_stop")
	pv_recorder_read_func				= lib.NewProc(lib, C.CString("pv_recorder_read"))
	pv_recorder_get_audio_devices_func 	= lib.NewProc("pv_recorder_get_audio_devices")
	pv_recorder_free_device_list_func 	= lib.NewProc("pv_recorder_free_device_list")
)

func (np nativePVRecorderType) nativeInit(pvrecorder *PVRecorder) PVRecorderStatus {
	var (
		deviceIndex = pvrecorder.DeviceIndex
		frameLength = pvrecorder.FrameLength
		userData 	= pvrecorder.userData
	)

	ret, _, _ := pv_recorder_init_func.Call(
		uintptr(deviceIndex),
		uintptr(frameLength),
		uintptr(unsafe.Pointer(C.callbackHandler)),
		uintptr(unsafe.Pointer(userData)),
		uintptr(unsafe.Pointer(&pvrecorder.handle))
	)

	return PVRecorderStatus(ret)
}

func (np nativePVRecorderType) nativeDelete(pvrecorder *PVRecorder) {
	pv_recorder_delete_func(pvrecorder.handle)
}

func (np nativePVRecorderType) nativeStart(pvrecorder *PVRecorder) PVRecorderStatus {
	ret, _, _ := pv_recorder_start_func(pvrecorder.handle)
	return PVRecorderStatus(ret)
}

func (np nativePVRecorderType) nativeStop(pvrecorder *PVRecorder) PVRecorderStatus {
	ret, _, _ := pv_recorder_stop_func(pvrecorder.handle)
	return PVRecorderStatus(ret)
}

func (np nativePVRecorderType) nativeRead(pvrecorder *PVRecorder, pcm unsafe.Pointer, length *int) PVRecorderStatus {
	ret, _, _ := pv_recorder_read_func(pvrecorder.handle,
		uintptr(pcm),
		uintptr(unsafe.Pointer(length)))
		
	return PVRecorderStatus(ret)
}

func nativeGetAudioDevices(count *int, devices ***C.char) PVRecorderStatus {
	ret, _, _ := pv_recorder_get_audio_devices_func(
		uintptr(unsafe.Pointer(count)),
		uintptr(unsafe.Pointer(devices)))

	return PVRecorderStatus(ret)
}

func nativeFreeDeviceList(count int, devices **C.char) {
	pv_recorder_free_device_list_func(
		uintptr(count),
		uintptr(unsafe.Pointer(devices)))
}
