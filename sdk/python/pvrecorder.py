#
# Copyright 2021 Picovoice Inc.
#
# You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
# file accompanying this source.
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
# an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.
#

import os
import platform
import subprocess
from ctypes import *
from enum import Enum

CALLBACK = CFUNCTYPE(None, POINTER(c_int16))


class PVRecorder(object):
    """
    A cross platform Python SDK for PV_Recorder to process audio recordings. It lists the available
    input devices. Also given the audio device index and frame_length, processes the frame and runs
    a callback each time a frame_length is given.
    """

    class PVRecorderStatuses(Enum):
        SUCCESS = 0
        OUT_OF_MEMORY = 1
        INVALID_ARGUMENT = 2
        INVALID_STATE = 3
        BACKEND_ERROR = 4
        DEVICE_ALREADY_INITIALIZED = 5
        DEVICE_NOT_INITIALIZED = 6
        RUNTIME_ERROR = 7

    _PVRECORDER_STATUS_TO_EXCEPTION = {
        PVRecorderStatuses.OUT_OF_MEMORY: MemoryError,
        PVRecorderStatuses.INVALID_ARGUMENT: ValueError,
        PVRecorderStatuses.INVALID_STATE: ValueError,
        PVRecorderStatuses.BACKEND_ERROR: SystemError,
        PVRecorderStatuses.DEVICE_ALREADY_INITIALIZED: ValueError,
        PVRecorderStatuses.DEVICE_NOT_INITIALIZED: ValueError,
        PVRecorderStatuses.RUNTIME_ERROR: RuntimeError
    }

    class CPVRecorder(Structure):
        pass

    _CALLBACK_TYPE = CFUNCTYPE(None, POINTER(c_int16))
    _LIBRARY = None

    def __init__(self, device_index, frame_length, callback):
        """
        Constructor

        :param device_index: The device index of the audio device to use. A (-1) will choose default audio device.
        :param frame_length: The length of the pcm frames to receive in each callback.
        :param callback: The function to run after a buffer of size 'frame_length' is processed.
        """

        if self._LIBRARY is None:
            self._LIBRARY = cdll.LoadLibrary(PVRecorder._lib_path())

        init_func = self._LIBRARY.pv_recorder_init
        init_func.argtypes = [
            c_int32,
            c_int32,
            self._CALLBACK_TYPE,
            POINTER(POINTER(self.CPVRecorder))
        ]
        init_func.restype = self.PVRecorderStatuses

        self._handle = POINTER(self.CPVRecorder)()
        self._callback = callback
        self._frame_length = frame_length

        status = init_func(device_index, frame_length, self._callback_wrapper(), byref(self._handle))
        if status is not self.PVRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to initialize pv_recorder.")

        self._delete_func = self._LIBRARY.pv_recorder_delete
        self._delete_func.argtypes = [POINTER(self.CPVRecorder)]
        self._delete_func.restype = None

        self._start_func = self._LIBRARY.pv_recorder_start
        self._start_func.argtypes = [POINTER(self.CPVRecorder)]
        self._start_func.restype = self.PVRecorderStatuses

        self._stop_func = self._LIBRARY.pv_recorder_stop
        self._stop_func.argtypes = [POINTER(self.CPVRecorder)]
        self._stop_func.restype = self.PVRecorderStatuses

    def delete(self):
        """Releases any resources used by PV_Recorder."""

        self._delete_func(self._handle)

    def start(self):
        """Starts recording audio."""

        status = self._start_func(self._handle)
        if status is not self.PVRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to start device.")

    def stop(self):
        """Stops recording audio."""

        status = self._stop_func(self._handle)
        if status is not self.PVRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to stop device.")

    def _callback_wrapper(self):
        """A callback wrapper which wraps the callback given from caller. The caller should expect an
        array of 'frame_length' as its argument."""

        def func(pcm):
            pcm_array = pcm[0:self._frame_length]
            self._callback(pcm_array)
        self._func = CALLBACK(func)
        return self._func

    @staticmethod
    def get_audio_devices():
        """Gets the audio devices currently available on device.

        :return: A list of strings, indicating the names of audio devices.
        """

        if PVRecorder._LIBRARY is None:
            PVRecorder._LIBRARY = cdll.LoadLibrary(PVRecorder._lib_path())

        get_audio_devices_func = PVRecorder._LIBRARY.pv_recorder_get_audio_devices
        get_audio_devices_func.argstype = [POINTER(c_int32), POINTER(POINTER(c_char_p))]
        get_audio_devices_func.restype = PVRecorder.PVRecorderStatuses

        free_device_list_func = PVRecorder._LIBRARY.pv_recorder_free_device_list
        free_device_list_func.argstype = [c_int32, POINTER(c_char_p)]
        free_device_list_func.restype = None

        count = c_int32()
        devices = POINTER(c_char_p)()

        status = get_audio_devices_func(byref(count), byref(devices))
        if status is not PVRecorder.PVRecorderStatuses.SUCCESS:
            raise PVRecorder._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to get device list")

        device_list = list()
        for i in range(count.value):
            device_list.append(devices[i].decode('utf-8'))

        free_device_list_func(count, devices)

        return device_list

    @staticmethod
    def _lib_path():
        """A helper function to get the library path."""

        if platform.system() == "Windows":
            script_path = os.path.join(os.path.dirname(__file__), "scripts", "platform.bat")
        else:
            script_path = os.path.join(os.path.dirname(__file__), "scripts", "platform.sh")

        command = subprocess.run(script_path, text=True, capture_output=True)

        if command.returncode != 0:
            raise RuntimeError("Current system is not supported.")
        os_name, cpu = str(command.stdout).split(" ")

        if os_name == "windows":
            extension = "dll"
        elif os_name == "mac":
            extension = "dylib"
        else:
            extension = "so"

        return os.path.join(os.path.dirname(__file__), "lib", os_name, cpu, f"libpv_recorder.{extension}")
