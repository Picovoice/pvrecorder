#
# Copyright 2021-2022 Picovoice Inc.
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


class PvRecorder(object):
    """
    A cross platform Python SDK for PvRecorder to process audio recordings. It lists the available
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

    def __init__(self, device_index, frame_length, buffer_size_msec=1000, log_overflow=True, log_silence=True):
        """
        Constructor

        :param device_index: The device index of the audio device to use. A (-1) will choose default audio device.
        :param frame_length: The length of the frame to receive at each read call.
        :param buffer_size_msec: Time in milliseconds indicating the total amount of time to store audio frames.
        :param log_overflow: Boolean variable to indicate to log overflow warnings. A log warning should indicate
        read is not being called fast enough from the callers point.
        :param log_silence: Boolean variable to enable silence logs. This will log when continuous audio buffers
        are detected as silent.
        """

        init_func = self._LIBRARY.pv_recorder_init
        init_func.argtypes = [
            c_int32,
            c_int32,
            c_int32,
            c_bool,
            c_bool,
            POINTER(POINTER(self.CPvRecorder))
        ]
        init_func.restype = self.PvRecorderStatuses

        self._handle = POINTER(self.CPvRecorder)()
        self._frame_length = frame_length

        status = init_func(device_index, frame_length, buffer_size_msec, log_overflow, log_silence, byref(self._handle))
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to initialize pv_recorder.")

        self._delete_func = self._LIBRARY.pv_recorder_delete
        self._delete_func.argtypes = [POINTER(self.CPvRecorder)]
        self._delete_func.restype = None

        self._start_func = self._LIBRARY.pv_recorder_start
        self._start_func.argtypes = [POINTER(self.CPvRecorder)]
        self._start_func.restype = self.PvRecorderStatuses

        self._stop_func = self._LIBRARY.pv_recorder_stop
        self._stop_func.argtypes = [POINTER(self.CPvRecorder)]
        self._stop_func.restype = self.PvRecorderStatuses

        self._read_func = self._LIBRARY.pv_recorder_read
        self._read_func.argtypes = [POINTER(self.CPvRecorder), POINTER(c_int16)]
        self._read_func.restype = self.PvRecorderStatuses

        self._get_selected_device_func = self._LIBRARY.pv_recorder_get_selected_device
        self._get_selected_device_func.argtypes = [POINTER(self.CPvRecorder)]
        self._get_selected_device_func.restype = c_char_p

        self._version_func = PvRecorder._LIBRARY.pv_recorder_version
        self._version_func.argtypes = None
        self._version_func.restype = c_char_p

    def delete(self):
        """Releases any resources used by PV_Recorder."""

        self._delete_func(self._handle)

    def start(self):
        """Starts recording audio."""

        status = self._start_func(self._handle)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to start device.")

    def stop(self):
        """Stops recording audio."""

        status = self._stop_func(self._handle)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to stop device.")

    def read(self):
        """Reads audio frames and returns a list containing the audio frames."""

        pcm = (c_int16 * self._frame_length)()
        status = self._read_func(self._handle, pcm)
        if status is not self.PvRecorderStatuses.SUCCESS:
            raise self._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to read from device.")
        return pcm[0:self._frame_length]

    @property
    def selected_device(self):
        """Gets the current selected device."""

        device_name = self._get_selected_device_func(self._handle)
        return device_name.decode('utf-8')

    @property
    def version(self):
        """Gets the current version of pv_recorder library."""

        version = self._version_func()
        return version.decode('utf-8')

    @staticmethod
    def get_audio_devices():
        """Gets the audio devices currently available on device.

        :return: A list of strings, indicating the names of audio devices.
        """

        get_audio_devices_func = PvRecorder._LIBRARY.pv_recorder_get_audio_devices
        get_audio_devices_func.argstype = [POINTER(c_int32), POINTER(POINTER(c_char_p))]
        get_audio_devices_func.restype = PvRecorder.PvRecorderStatuses

        free_device_list_func = PvRecorder._LIBRARY.pv_recorder_free_device_list
        free_device_list_func.argstype = [c_int32, POINTER(c_char_p)]
        free_device_list_func.restype = None

        count = c_int32()
        devices = POINTER(c_char_p)()

        status = get_audio_devices_func(byref(count), byref(devices))
        if status is not PvRecorder.PvRecorderStatuses.SUCCESS:
            raise PvRecorder._PVRECORDER_STATUS_TO_EXCEPTION[status]("Failed to get device list")

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

        return os.path.join(os.path.dirname(__file__), "lib", os_name, cpu, "libpv_recorder.%s" % extension)

    _LIBRARY = cdll.LoadLibrary(_lib_path.__func__())
