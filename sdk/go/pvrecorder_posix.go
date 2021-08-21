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

typedef int32_t (*pv_recorder_init_func)(int32_t, int32_t, void (*)(int16_t *))

int32_t pv_recorder_init_wrapper(void *f, int32_t device_index, int32_t frame_length, void(*callback)(int16_t *)) {
	return ((pv_recorder_init_func) f)(device_index, frame_length, callback);
}

typedef void (*pv_recorder_delete_func)(void *);

void pv_recorder_delete_wrapper(void *f, void *object) {
	return ((pv_recorder_delete_func) f)(object);
}

typedef int32_t (*pv_recorder_start_func)(void *);

void pv_recorder_start_wrapper(void *f, void *object) {
	return ((ov_recorder_start_func) f)(object);
}

typedef int32_t (*pv_recorder_stop_func)(void *);

void pv_recorder_stop_wrapper(void *f, void *object) {
	return ((ov_recorder_stop_func) f)(object);
}

typedef int32_t (*pv_recorder_get_audio_devices_func)(int32_t *, char ***);

void pv_recorder_get_audio_devices_wrapper(void *f, int32_t *count, char ***devices) {
	return ((pv_recorder_get_audio_devices_func) f)(object, count, devices);
}

typedef void (*pv_recorder_free_device_list)(int32_t, char**);

void pv_recorder_get_free_device_list(void *f, int32_t count, char **devices) {
	return ((pv_recorder_free_device_list) f)(object, count, devices);
}

void *pv_recorder_callback_wrapper(void *callback) {
	return 
}

*/
import "C"

import (
	"unsafe"
)

var (
	lib = C.dlopen(C.CString(getLibPath()), C.RTLD_NOW)

	pv_recorder_init_ptr = C.dlsym(lib, C.CString("pv_recorder_init"))
	pv_recorder_delete_ptr = C.dlsym(lib, C.CString("pv_recorder_delete"))
	pv_recorder_start_ptr = C.dlsym(lib, C.CString("pv_recorder_start"))
	pv_recorder_stop_ptr = C.dlsym(lib, C.CString("pv_recorder_stop"))
	pv_recorder_get_audio_devices_ptr = C.dlsym(lib, C.CString("pv_recorder_get_audio_devices"))
	pv_recorder_free_device_list_ptr = C.dlsym(lib, C.CString("pv_recorder_free_device_list"))
)

func (nr nativePVRecorderType) nativeInit(pvrecorder *PVRecorder) (status PVRecorderStatus) {
	var (
		deviceIndex = pvrecorder.deviceIndex
		frameLength = pvrecorder.frameLength
		callback = pvrecorder.callback
	)

	var ret = C.pv_recorder_init_wrapper(pv_recorder_init_ptr, 
		(C.int32_t) deviceIndex, 
		(C.int32_t) frameLength,
		() callback)
	

}
