#
# Copyright 2021-2023 Picovoice Inc.
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
from typing import *

CALLBACK = CFUNCTYPE(None, POINTER(c_int16))


def default_library_path(relative: str = ''):
    """A helper function to get the library path."""

    if platform.system() == "Windows":
        script_path = os.path.join(os.path.dirname(__file__), relative, "resources", "scripts", "platform.bat")
    else:
        script_path = os.path.join(os.path.dirname(__file__), relative, "resources", "scripts", "platform.sh")

    command = subprocess.run(script_path, stdout=subprocess.PIPE)

    if command.returncode != 0:
        raise RuntimeError("Current system is not supported.")
    os_name, cpu = str(command.stdout.decode("utf-8")).split(" ")

    if os_name == "windows":
        extension = "dll"
    elif os_name == "mac":
        extension = "dylib"
    else:
        extension = "so"

    return os.path.join(os.path.dirname(__file__), relative, "lib", os_name, cpu, "libpv_recorder.%s" % extension)


class PvRecorder(object):
    """
    A cross-platform Python SDK for PvRecorder to process audio recordings. It lists the available
    input devices. Also given the audio device index and frame_length, processes the frame and runs
    a callback each time a frame_length is given.
    """

    class PvRecorderStatuses(Enum):
        SUCCESS = 0
        OUT_OF_MEMORY = 1
        INVALID_ARGUMENT = 2
        INVALID_STATE = 3
        BACKEND_ERROR = 4
        DEVICE_ALREADY_INITIALIZED = 5
        DEVICE_NOT_INITIALIZED = 6
        IO_ERROR = 7
        RUNTIME_ERROR = 8

    _PVRECORDER_STATUS_TO_EXCEPTION = {
        PvRecorderStatuses.OUT_OF_MEMORY: MemoryError,
        PvRecorderStatuses.INVALID_ARGUMENT: ValueError,
        PvRecorderStatuses.INVALID_STATE: ValueError,
        PvRecorderStatuses.BACKEND_ERROR: SystemError,
        PvRecorderStatuses.DEVICE_ALREADY_INITIALIZED: ValueError,
        PvRecorderStatuses.DEVICE_NOT_INITIALIZED: ValueError,
        PvRecorderStatuses.IO_ERROR: IOError,
        PvRecorderStatuses.RUNTIME_ERROR: RuntimeError
    }

    class CPvRecorder(Structure):
        pass

    _library = None
    _relative_library_path = ''

    def __init__(
            self,
            frame_length: int,
            device_index: int = -1,
            buffered_frames_count: int = 50):
        """
        Constructor

        :param frame_length: The length of audio frame to get for each read call.
        :param device_index: The index of the audio device to use. A value of (-1) will resort to default device.
        :param buffered_frames_count: The number of audio frames buffered internally for reading - i.e. internal
        circular buffer will be of size `frame_length` * `buffered_frames_count`. If this value is too low,
        buffer overflows could occur audio frames could be dropped. A higher value will increase memory usage.
        """

        library = self._get_library()

        init_func = library.pv_recorder_init
        init_func.argtypes = [
            c_int32,
            c_int32,
            c_int32,
            POINTER(POINTER(self.CPvRecorder))
        ]
        init_func.restype = self.PvRecorderStatuses

        self._handle = POINTER(self.CPvRecorder)()
        self._frame_length = frame_length

        status = init_func(frame_length, device_index, buffered_frames_count, byref(self._handle))
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to initialize PvRecorder.")

        self._delete_func = library.pv_recorder_delete
        self._delete_func.argtypes = [POINTER(self.CPvRecorder)]
        self._delete_func.restype = None

        self._start_func = library.pv_recorder_start
        self._start_func.argtypes = [POINTER(self.CPvRecorder)]
        self._start_func.restype = self.PvRecorderStatuses

        self._stop_func = library.pv_recorder_stop
        self._stop_func.argtypes = [POINTER(self.CPvRecorder)]
        self._stop_func.restype = self.PvRecorderStatuses

        self._set_debug_logging_func = library.pv_recorder_set_debug_logging
        self._set_debug_logging_func.argtypes = [POINTER(self.CPvRecorder), c_bool]
        self._set_debug_logging_func.restype = None

        self._read_func = library.pv_recorder_read
        self._read_func.argtypes = [POINTER(self.CPvRecorder), POINTER(c_int16)]
        self._read_func.restype = self.PvRecorderStatuses

        self._get_is_recording_func = library.pv_recorder_get_is_recording
        self._get_is_recording_func.argtypes = [POINTER(self.CPvRecorder)]
        self._get_is_recording_func.restype = c_bool

        self._get_selected_device_func = library.pv_recorder_get_selected_device
        self._get_selected_device_func.argtypes = [POINTER(self.CPvRecorder)]
        self._get_selected_device_func.restype = c_char_p

        self._version_func = library.pv_recorder_version
        self._version_func.argtypes = None
        self._version_func.restype = c_char_p

        self._sample_rate_func = library.pv_recorder_sample_rate
        self._sample_rate_func.argtypes = None
        self._sample_rate_func.restype = c_int32

    def delete(self) -> None:
        """Releases any resources used by PvRecorder."""

        self._delete_func(self._handle)

    def start(self) -> None:
        """Starts recording and buffering audio frames."""

        status = self._start_func(self._handle)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to start device.")

    def stop(self) -> None:
        """Stops recording audio."""

        status = self._stop_func(self._handle)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to stop device.")

    def read(self) -> List[int]:
        """Synchronous call to read a frame of audio.

        :return: A frame with size `frame_length` matching the value given to `__init__()`.
        """

        pcm = (c_int16 * self._frame_length)()
        status = self._read_func(self._handle, pcm)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to read from device.")
        return list(pcm[0:self._frame_length])

    def set_debug_logging(self, is_debug_logging_enabled: bool) -> None:
        """
        Enable or disable debug logging for PvRecorder. Debug logs will indicate when there are overflows
        in the internal frame buffer and when an audio source is generating frames of silence.

        :param is_debug_logging_enabled: Boolean indicating whether the debug logging is enabled or disabled.
        """

        self._set_debug_logging_func(self._handle, is_debug_logging_enabled)

    @property
    def is_recording(self) -> bool:
        """Gets whether the recorder is currently recording audio or not."""

        return bool(self._get_is_recording_func(self._handle))

    @property
    def selected_device(self) -> str:
        """Gets the audio device that the given `PvRecorder` instance is using."""

        device_name = self._get_selected_device_func(self._handle)
        return device_name.decode('utf-8')

    @property
    def version(self) -> str:
        """Gets the current version of PvRecorder library."""

        version = self._version_func()
        return version.decode('utf-8')

    @property
    def frame_length(self) -> int:
        """Gets the frame length matching the value given to `__init__()`."""

        return self._frame_length

    @property
    def sample_rate(self) -> int:
        """Gets the audio sample rate used by PvRecorder."""

        sample_rate = self._sample_rate_func()
        return sample_rate

    @staticmethod
    def get_available_devices() -> List[str]:
        """Gets the list of available audio devices that can be used for recording.

        :return: A list of strings, indicating the names of audio devices.
        """

        get_available_devices_func = PvRecorder._get_library().pv_recorder_get_available_devices
        get_available_devices_func.argstype = [POINTER(c_int32), POINTER(POINTER(c_char_p))]
        get_available_devices_func.restype = PvRecorder.PvRecorderStatuses

        free_available_devices_func = PvRecorder._get_library().pv_recorder_free_available_devices
        free_available_devices_func.argstype = [c_int32, POINTER(c_char_p)]
        free_available_devices_func.restype = None

        count = c_int32()
        devices = POINTER(c_char_p)()

        status = get_available_devices_func(byref(count), byref(devices))
        if status is not PvRecorder.PvRecorderStatuses.SUCCESS:
            raise PvRecorder._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to get device list")

        device_list = list()
        for i in range(count.value):
            device_list.append(devices[i].decode('utf-8'))

        free_available_devices_func(count, devices)

        return device_list

    @classmethod
    def set_default_library_path(cls, relative: str):
        cls._relative_library_path = default_library_path(relative)

    @classmethod
    def _get_library(cls):
        if len(cls._relative_library_path) == 0:
            cls._relative_library_path = default_library_path()
        if cls._library is None:
            cls._library = cdll.LoadLibrary(cls._relative_library_path)
        return cls._library


__all__ = [
    'PvRecorder',
]
