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

// +build linux darwin

package pvrecorder

/*
#cgo LDFLAGS: -lpthread -ldl -lm
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>

extern void callbackHandler(int16_t *pcm, void *userData);

typedef int32_t (*pv_recorder_init_func)(int32_t, int32_t, void (*)(int16_t *, void *), void *, void **);

int32_t pv_recorder_init_wrapper(void *f, int32_t device_index, int32_t frame_length, void *user_data, void **object) {
	return ((pv_recorder_init_func) f)(device_index, frame_length, callbackHandler, user_data, object);
}

typedef void (*pv_recorder_delete_func)(void *);

void pv_recorder_delete_wrapper(void *f, void *object) {
	return ((pv_recorder_delete_func) f)(object);
}

typedef int32_t (*pv_recorder_start_func)(void *);

int32_t pv_recorder_start_wrapper(void *f, void *object) {
	return ((pv_recorder_start_func) f)(object);
}

typedef int32_t (*pv_recorder_stop_func)(void *);

int32_t pv_recorder_stop_wrapper(void *f, void *object) {
	return ((pv_recorder_stop_func) f)(object);
}

typedef int32_t (*pv_recorder_get_audio_devices_func)(int32_t *, char ***);

int32_t pv_recorder_get_audio_devices_wrapper(void *f, int32_t *count, char ***devices) {
	return ((pv_recorder_get_audio_devices_func) f)(count, devices);
}

typedef void (*pv_recorder_free_device_list_func)(int32_t, char**);

void pv_recorder_free_device_list_wrapper(void *f, int32_t count, char **devices) {
	return ((pv_recorder_free_device_list_func) f)(count, devices);
}

*/
import "C"

import (
	"unsafe"
)

// private vars
var (
	lib = C.dlopen(C.CString(libName), C.RTLD_NOW)

	pv_recorder_init_ptr 				= C.dlsym(lib, C.CString("pv_recorder_init"))
	pv_recorder_delete_ptr 				= C.dlsym(lib, C.CString("pv_recorder_delete"))
	pv_recorder_start_ptr 				= C.dlsym(lib, C.CString("pv_recorder_start"))
	pv_recorder_stop_ptr 				= C.dlsym(lib, C.CString("pv_recorder_stop"))
	pv_recorder_get_audio_devices_ptr 	= C.dlsym(lib, C.CString("pv_recorder_get_audio_devices"))
	pv_recorder_free_device_list_ptr 	= C.dlsym(lib, C.CString("pv_recorder_free_device_list"))
)

func (np nativePVRecorderType) nativeInit(pvrecorder *PVRecorder) PVRecorderStatus {
	var (
		deviceIndex = pvrecorder.DeviceIndex
		frameLength = pvrecorder.FrameLength
		userData 	= pvrecorder.userData
		ptrC 		= make([]unsafe.Pointer, 1)
	)
	
	var ret = C.pv_recorder_init_wrapper(pv_recorder_init_ptr,
		(C.int32_t)(deviceIndex),
		(C.int32_t)(frameLength),
		(unsafe.Pointer)(userData),
		&ptrC[0])

	pvrecorder.handle = uintptr(ptrC[0])
	return PVRecorderStatus(ret)
}

func (nativePVRecorderType) nativeDelete(pvrecorder *PVRecorder) {
	C.pv_recorder_delete_wrapper(pv_recorder_delete_ptr,
		unsafe.Pointer(pvrecorder.handle))
}

func (nativePVRecorderType) nativeStart(pvrecorder *PVRecorder) PVRecorderStatus {
	var ret = C.pv_recorder_start_wrapper(pv_recorder_start_ptr,
		unsafe.Pointer(pvrecorder.handle))

	return PVRecorderStatus(ret)
}

func (nativePVRecorderType) nativeStop(pvrecorder *PVRecorder) PVRecorderStatus {
	var ret = C.pv_recorder_stop_wrapper(pv_recorder_stop_ptr,
		unsafe.Pointer(pvrecorder.handle))
	
	return PVRecorderStatus(ret)
}

func nativeGetAudioDevices(count *int, devices ***C.char) PVRecorderStatus {
	var ret = C.pv_recorder_get_audio_devices_wrapper(pv_recorder_get_audio_devices_ptr,
		(*C.int32_t)(unsafe.Pointer(count)),
		(***C.char)(unsafe.Pointer(devices)))

	return PVRecorderStatus(ret)
}

func nativeFreeDeviceList(count int, devices **C.char) {
	C.pv_recorder_free_device_list_wrapper(pv_recorder_free_device_list_ptr,
		(C.int32_t)(count),
		(**C.char)(unsafe.Pointer(devices)))
}
