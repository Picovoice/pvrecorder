// Copyright 2021-2023 Picovoice Inc.
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
#include <stdbool.h>
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

typedef int32_t (*pv_recorder_init_func)(int32_t, int32_t, int32_t, void **);

int32_t pv_recorder_init_wrapper(void *f, int32_t frame_length, int32_t device_index, int32_t buffered_frames_count, void **object) {
    return ((pv_recorder_init_func) f)(frame_length, device_index, buffered_frames_count, object);
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

typedef void (*pv_recorder_set_debug_logging_func)(void *, bool);

void pv_recorder_set_debug_logging_wrapper(void *f, void *object, bool is_debug_logging_enabled) {
	return ((pv_recorder_set_debug_logging_func) f)(object, is_debug_logging_enabled);
}

typedef bool (*pv_recorder_get_is_recording_func)(void *);

bool pv_recorder_get_is_recording_wrapper(void *f, void* object) {
    return ((pv_recorder_get_is_recording_func) f)(object);
}

typedef const char *(*pv_recorder_get_selected_device_func)(void *);

const char *pv_recorder_get_selected_device_wrapper(void *f, void* object) {
    return ((pv_recorder_get_selected_device_func) f)(object);
}

typedef int32_t (*pv_recorder_get_available_devices_func)(int32_t *, char ***);

int32_t pv_recorder_get_available_devices_wrapper(void *f, int32_t *count, char ***devices) {
    return ((pv_recorder_get_available_devices_func) f)(count, devices);
}

typedef void (*pv_recorder_free_available_devices_func)(int32_t, char**);

void pv_recorder_free_available_devices_wrapper(void *f, int32_t count, char **devices) {
    return ((pv_recorder_free_available_devices_func) f)(count, devices);
}

typedef const char *(*pv_recorder_version_func)();

const char *pv_recorder_version_wrapper(void *f) {
    return ((pv_recorder_version_func) f)();
}

typedef int32_t (*pv_recorder_sample_rate_func)();

int32_t pv_recorder_sample_rate_wrapper(void *f) {
    return ((pv_recorder_sample_rate_func) f)();
}

*/
import "C"

import (
	"unsafe"
)

// private vars
var (
	lib = C.open_dl(C.CString(libName))

	pv_recorder_init_ptr                   = C.load_symbol(lib, C.CString("pv_recorder_init"))
	pv_recorder_delete_ptr                 = C.load_symbol(lib, C.CString("pv_recorder_delete"))
	pv_recorder_start_ptr                  = C.load_symbol(lib, C.CString("pv_recorder_start"))
	pv_recorder_stop_ptr                   = C.load_symbol(lib, C.CString("pv_recorder_stop"))
	pv_recorder_read_ptr                   = C.load_symbol(lib, C.CString("pv_recorder_read"))
	pv_recorder_set_debug_logging_ptr      = C.load_symbol(lib, C.CString("pv_recorder_set_debug_logging"))
	pv_recorder_get_is_recording_ptr       = C.load_symbol(lib, C.CString("pv_recorder_get_is_recording"))
	pv_recorder_get_selected_device_ptr    = C.load_symbol(lib, C.CString("pv_recorder_get_selected_device"))
	pv_recorder_get_available_devices_ptr  = C.load_symbol(lib, C.CString("pv_recorder_get_available_devices"))
	pv_recorder_free_available_devices_ptr = C.load_symbol(lib, C.CString("pv_recorder_free_available_devices"))
	pv_recorder_version_ptr                = C.load_symbol(lib, C.CString("pv_recorder_version"))
	pv_recorder_sample_rate_ptr            = C.load_symbol(lib, C.CString("pv_recorder_sample_rate"))
)

func (np nativePvRecorderType) nativeInit(pvRecorder *PvRecorder) PvRecorderStatus {
	var (
		frameLength         = pvRecorder.FrameLength
		deviceIndex         = pvRecorder.DeviceIndex
		bufferedFramesCount = pvRecorder.BufferedFramesCount
		ptrC                = make([]unsafe.Pointer, 1)
	)

	var ret = C.pv_recorder_init_wrapper(pv_recorder_init_ptr,
		(C.int32_t)(frameLength),
		(C.int32_t)(deviceIndex),
		(C.int32_t)(bufferedFramesCount),
		&ptrC[0])

	pvRecorder.handle = uintptr(ptrC[0])
	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeDelete(pvRecorder *PvRecorder) {
	C.pv_recorder_delete_wrapper(pv_recorder_delete_ptr,
		unsafe.Pointer(pvRecorder.handle))
}

func (nativePvRecorderType) nativeStart(pvRecorder *PvRecorder) PvRecorderStatus {
	var ret = C.pv_recorder_start_wrapper(pv_recorder_start_ptr,
		unsafe.Pointer(pvRecorder.handle))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeStop(pvRecorder *PvRecorder) PvRecorderStatus {
	var ret = C.pv_recorder_stop_wrapper(pv_recorder_stop_ptr,
		unsafe.Pointer(pvRecorder.handle))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeRead(pvRecorder *PvRecorder, pcm *C.int16_t) PvRecorderStatus {
	var ret = C.pv_recorder_read_wrapper(pv_recorder_read_ptr,
		unsafe.Pointer(pvRecorder.handle),
		pcm)

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeSetDebugLogging(pvRecorder *PvRecorder, isDebugLoggingEnabled bool) {
	C.pv_recorder_set_debug_logging_wrapper(pv_recorder_set_debug_logging_ptr,
		unsafe.Pointer(pvRecorder.handle),
		C.bool(isDebugLoggingEnabled))
}

func (nativePvRecorderType) nativeGetIsRecording(pvRecorder *PvRecorder) bool {
	var ret = C.pv_recorder_get_is_recording_wrapper(pv_recorder_get_is_recording_ptr,
		unsafe.Pointer(pvRecorder.handle))

	return bool(ret)
}

func (nativePvRecorderType) nativeGetSelectedDevice(pvRecorder *PvRecorder) string {
	var ret = C.pv_recorder_get_selected_device_wrapper(pv_recorder_get_selected_device_ptr,
		unsafe.Pointer(pvRecorder.handle))

	return C.GoString(ret)
}

func (nativePvRecorderType) nativeGetAudioDevices(count *int, devices ***C.char) PvRecorderStatus {
	var ret = C.pv_recorder_get_available_devices_wrapper(pv_recorder_get_available_devices_ptr,
		(*C.int32_t)(unsafe.Pointer(count)),
		(***C.char)(unsafe.Pointer(devices)))

	return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeFreeDeviceList(count int, devices **C.char) {
	C.pv_recorder_free_available_devices_wrapper(pv_recorder_free_available_devices_ptr,
		(C.int32_t)(count),
		(**C.char)(unsafe.Pointer(devices)))
}

func (nativePvRecorderType) nativeVersion() string {
	var ret = C.pv_recorder_version_wrapper(pv_recorder_version_ptr)
	return C.GoString(ret)
}

func (nativePvRecorderType) nativeSampleRate() int {
	var ret = C.pv_recorder_sample_rate_wrapper(pv_recorder_sample_rate_ptr)
	return int(ret)
}
