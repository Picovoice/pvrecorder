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
*/
import "C"

import (
    "unsafe"

    "golang.org/x/sys/windows"
)

// private vars
var (
    lib = windows.NewLazyDLL(libName)

    pv_recorder_init_func                   = lib.NewProc("pv_recorder_init")
    pv_recorder_delete_func                 = lib.NewProc("pv_recorder_delete")
    pv_recorder_start_func                  = lib.NewProc("pv_recorder_start")
    pv_recorder_stop_func                   = lib.NewProc("pv_recorder_stop")
    pv_recorder_read_func                   = lib.NewProc("pv_recorder_read")
    pv_recorder_get_selected_device_func    = lib.NewProc("pv_recorder_get_selected_device")
    pv_recorder_get_audio_devices_func      = lib.NewProc("pv_recorder_get_audio_devices")
    pv_recorder_free_device_list_func       = lib.NewProc("pv_recorder_free_device_list")
    pv_recorder_version_func                = lib.NewProc("pv_recorder_version")
)

func (np nativePvRecorderType) nativeInit(pvrecorder *PvRecorder) PvRecorderStatus {
    var (
        deviceIndex     = pvrecorder.DeviceIndex
        frameLength     = pvrecorder.FrameLength
        bufferSizeMSec  = pvrecorder.BufferSizeMSec
        logOverflow     = pvrecorder.LogOverflow
    )

    ret, _, _ := pv_recorder_init_func.Call(
        uintptr(deviceIndex),
        uintptr(frameLength),
        uintptr(bufferSizeMSec),
        uintptr(logOverflow),
        uintptr(unsafe.Pointer(&pvrecorder.handle)))

    return PvRecorderStatus(ret)
}

func (np nativePvRecorderType) nativeDelete(pvrecorder *PvRecorder) {
    pv_recorder_delete_func.Call(pvrecorder.handle)
}

func (np nativePvRecorderType) nativeStart(pvrecorder *PvRecorder) PvRecorderStatus {
    ret, _, _ := pv_recorder_start_func.Call(pvrecorder.handle)

    return PvRecorderStatus(ret)
}

func (np nativePvRecorderType) nativeStop(pvrecorder *PvRecorder) PvRecorderStatus {
    ret, _, _ := pv_recorder_stop_func.Call(pvrecorder.handle)

    return PvRecorderStatus(ret)
}

func (np nativePvRecorderType) nativeRead(pvrecorder *PvRecorder, pcm *C.int16_t) PvRecorderStatus {
    ret, _, _ := pv_recorder_read_func.Call(pvrecorder.handle,
        uintptr(unsafe.Pointer(pcm)))

    return PvRecorderStatus(ret)
}

func (np nativePvRecorderType) nativeGetSelectedDevice(pvrecorder *PvRecorder) string {
    ret, _, _ := pv_recorder_get_selected_device_func.Call(pvrecorder.handle)

    return C.GoString((*C.char)(unsafe.Pointer(ret)))
}

func (nativePvRecorderType) nativeGetAudioDevices(count *int, devices ***C.char) PvRecorderStatus {
    ret, _, _ := pv_recorder_get_audio_devices_func.Call(
        uintptr(unsafe.Pointer(count)),
        uintptr(unsafe.Pointer(devices)))

    return PvRecorderStatus(ret)
}

func (nativePvRecorderType) nativeFreeDeviceList(count int, devices **C.char) {
    pv_recorder_free_device_list_func.Call(
        uintptr(count),
        uintptr(unsafe.Pointer(devices)))
}

func (nativePvRecorderType) nativeVersion() string {
    ret, _, _ := pv_recorder_version_func.Call()

    return C.GoString((*C.char)(unsafe.Pointer(ret)))
}
