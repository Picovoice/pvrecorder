// Copyright 2021-2022 Picovoice Inc.
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
#cgo linux LDFLAGS: -lpthread -ldl -lm
#cgo darwin LDFLAGS: -lpthread -ldl -lm
#include <stdlib.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

#else

    #include <dlfcn.h>

#endif

static void *open_dl(const char *dl_path) {

#if defined(_WIN32) || defined(_WIN64)

    return LoadLibrary((LPCSTR) dl_path);

#else

    return dlopen(dl_path, RTLD_NOW);

#endif

}

static void *load_symbol(void *handle, const char *symbol) {

#if defined(_WIN32) || defined(_WIN64)

    return GetProcAddress((HMODULE) handle, symbol);

#else

    return dlsym(handle, symbol);

#endif

}

typedef int32_t (*pv_recorder_init_func)(int32_t, int32_t, int32_t, int32_t, int32_t, void **);

int32_t pv_recorder_init_wrapper(void *f, int32_t device_index, int32_t frame_length, int32_t buffer_size_msec, int32_t log_overflow, int32_t log_silence, void **object) {
    return ((pv_recorder_init_func) f)(device_index, frame_length, buffer_size_msec, log_overflow, log_silence, object);
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

typedef int32_t (*pv_recorder_read_func)(void *, int16_t *);

int32_t pv_recorder_read_wrapper(void *f, void *object, int16_t *pcm) {
    return ((pv_recorder_read_func) f)(object, pcm);
}

typedef const char *(*pv_recorder_get_selected_device_func)(void *);

const char *pv_recorder_get_selected_device_wrapper(void *f, void* object) {
    return ((pv_recorder_get_selected_device_func) f)(object);
}

typedef int32_t (*pv_recorder_get_audio_devices_func)(int32_t *, char ***);

int32_t pv_recorder_get_audio_devices_wrapper(void *f, int32_t *count, char ***devices) {
    return ((pv_recorder_get_audio_devices_func) f)(count, devices);
}

typedef void (*pv_recorder_free_device_list_func)(int32_t, char**);

void pv_recorder_free_device_list_wrapper(void *f, int32_t count, char **devices) {
    return ((pv_recorder_free_device_list_func) f)(count, devices);
}

typedef const char *(*pv_recorder_version_func)();

const char *pv_recorder_version_wrapper(void *f) {
    return ((pv_recorder_version_func) f)();
}

*/
import "C"

import (
	"unsafe"
)

// private vars
var (
	lib = C.open_dl(C.CString(libName))

	pv_recorder_init_ptr                = C.load_symbol(lib, C.CString("pv_recorder_init"))
	pv_recorder_delete_ptr              = C.load_symbol(lib, C.CString("pv_recorder_delete"))
	pv_recorder_start_ptr               = C.load_symbol(lib, C.CString("pv_recorder_start"))
	pv_recorder_stop_ptr                = C.load_symbol(lib, C.CString("pv_recorder_stop"))
	pv_recorder_read_ptr                = C.load_symbol(lib, C.CString("pv_recorder_read"))
	pv_recorder_get_selected_device_ptr = C.load_symbol(lib, C.CString("pv_recorder_get_selected_device"))
	pv_recorder_get_audio_devices_ptr   = C.load_symbol(lib, C.CString("pv_recorder_get_audio_devices"))
	pv_recorder_free_device_list_ptr    = C.load_symbol(lib, C.CString("pv_recorder_free_device_list"))
	pv_recorder_version_ptr             = C.load_symbol(lib, C.CString("pv_recorder_version"))
)

func (np nativePvRecorderType) nativeInit(pvrecorder *PvRecorder) PvRecorderStatus {
	var (
		deviceIndex    = pvrecorder.DeviceIndex
		frameLength    = pvrecorder.FrameLength
		bufferSizeMSec = pvrecorder.BufferSizeMSec
		logOverflow    = pvrecorder.LogOverflow
		logSilence     = pvrecorder.LogSilence
		ptrC           = make([]unsafe.Pointer, 1)
	)

	var ret = C.pv_recorder_init_wrapper(pv_recorder_init_ptr,
		(C.int32_t)(deviceIndex),
		(C.int32_t)(frameLength),
		(C.int32_t)(bufferSizeMSec),
		(C.int32_t)(logOverflow),
		(C.int32_t)(logSilence),
		&ptrC[0])

	pvrecorder.handle = uintptr(ptrC[0])
	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeDelete(pvrecorder *PvRecorder) {
	C.pv_recorder_delete_wrapper(pv_recorder_delete_ptr,
		unsafe.Pointer(pvrecorder.handle))
}

func (nativePvRecorderType) nativeStart(pvrecorder *PvRecorder) PvRecorderStatus {
	var ret = C.pv_recorder_start_wrapper(pv_recorder_start_ptr,
		unsafe.Pointer(pvrecorder.handle))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeStop(pvrecorder *PvRecorder) PvRecorderStatus {
	var ret = C.pv_recorder_stop_wrapper(pv_recorder_stop_ptr,
		unsafe.Pointer(pvrecorder.handle))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeRead(pvrecorder *PvRecorder, pcm *C.int16_t) PvRecorderStatus {
	var ret = C.pv_recorder_read_wrapper(pv_recorder_read_ptr,
		unsafe.Pointer(pvrecorder.handle),
		pcm)

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeGetSelectedDevice(pvrecorder *PvRecorder) string {
	var ret = C.pv_recorder_get_selected_device_wrapper(pv_recorder_get_selected_device_ptr,
		unsafe.Pointer(pvrecorder.handle))

	return C.GoString(ret)
}

func (nativePvRecorderType) nativeGetAudioDevices(count *int, devices ***C.char) PvRecorderStatus {
	var ret = C.pv_recorder_get_audio_devices_wrapper(pv_recorder_get_audio_devices_ptr,
		(*C.int32_t)(unsafe.Pointer(count)),
		(***C.char)(unsafe.Pointer(devices)))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeFreeDeviceList(count int, devices **C.char) {
	C.pv_recorder_free_device_list_wrapper(pv_recorder_free_device_list_ptr,
		(C.int32_t)(count),
		(**C.char)(unsafe.Pointer(devices)))
}

func (nativePvRecorderType) nativeVersion() string {
	var ret = C.pv_recorder_version_wrapper(pv_recorder_version_ptr)
	return C.GoString(ret)
}
