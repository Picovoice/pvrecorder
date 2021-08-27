/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

use libc::c_char;
use libloading::{Library, Symbol};
use std::cmp::PartialEq;
use std::convert::AsRef;
use std::ffi::CStr;
use std::path::{Path, PathBuf};
use std::ptr::addr_of_mut;
use std::sync::Arc;

#[cfg(target_family = "unix")]
use libloading::os::unix::Symbol as RawSymbol;

#[cfg(target_family = "windows")]
use libloading::os::windows::Symbol as RawSymbol;

use crate::util::*;

#[repr(C)]
struct CPvRecorder {}

#[repr(C)]
#[derive(PartialEq, Debug)]
#[allow(non_camel_case_types)]
pub enum PvRecorderStatus {
    SUCCESS = 0,
    OUT_OF_MEMORY = 1,
    INVALID_ARGUMENT = 2,
    INVALID_STATE = 3,
    BACKEND_ERROR = 4,
    DEVICE_ALREADY_INITIALIZED = 5,
    DEVICE_NOT_INITIALIZED = 6,
    IO_ERROR = 7,
    BUFFER_OVERFLOW = 8,
    RUNTIME_ERROR = 9,
}

type PvRecorderInitFn = unsafe extern "C" fn(
    device_index: i32,
    frame_length: i32,
    milliseconds: i32,
    enable_logs: bool,
    object: *mut *mut CPvRecorder,
) -> PvRecorderStatus;
type PvRecorderDeleteFn = unsafe extern "C" fn(object: *mut CPvRecorder);
type PvRecorderStartFn = unsafe extern "C" fn(object: *mut CPvRecorder) -> PvRecorderStatus;
type PvRecorderStopFn = unsafe extern "C" fn(object: *mut CPvRecorder) -> PvRecorderStatus;
type PvRecorderReadFn =
    unsafe extern "C" fn(object: *mut CPvRecorder, pcm: *mut i16) -> PvRecorderStatus;
type PvRecorderGetAudioDevicesFn =
    unsafe extern "C" fn(count: *mut i32, devices: *mut *mut *mut c_char) -> PvRecorderStatus;
type PvRecorderFreeDeviceList = unsafe extern "C" fn(count: i32, devices: *mut *mut c_char);

#[derive(Debug)]
pub enum RecorderErrorStatus {
    LibraryError(PvRecorderStatus),
    LibraryLoadError,
    ArgumentError,
    OtherError,
}

#[derive(Debug)]
pub struct RecorderError {
    pub status: RecorderErrorStatus,
    pub message: Option<String>,
}

impl RecorderError {
    pub fn new(status: RecorderErrorStatus, message: &str) -> Self {
        RecorderError {
            status,
            message: Some(message.to_string()),
        }
    }
}

impl std::fmt::Display for RecorderError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match &self.message {
            Some(message) => write!(f, "{}: {:?}", message, self.status),
            None => write!(f, "Recorder error: {:?}", self.status),
        }
    }
}

const DEFAULT_DEVICE_INDEX: i32 = -1;
const DEFAULT_FRAME_LENGTH: i32 = 512;
const DEFAULT_MILLISECONDS: i32 = 1000;

pub struct RecorderBuilder {
    library_path: PathBuf,
    device_index: i32,
    frame_length: i32,
    milliseconds: i32,
    enable_logs: bool,
}

impl RecorderBuilder {
    pub fn new() -> Self {
        return Self {
            library_path: pv_library_path(),
            device_index: DEFAULT_DEVICE_INDEX,
            frame_length: DEFAULT_FRAME_LENGTH,
            milliseconds: DEFAULT_MILLISECONDS,
            enable_logs: false,
        };
    }

    pub fn device_index<'a>(&'a mut self, device_index: i32) -> &'a mut Self {
        self.device_index = device_index;
        return self;
    }

    pub fn frame_length<'a>(&'a mut self, frame_length: i32) -> &'a mut Self {
        self.frame_length = frame_length;
        return self;
    }

    pub fn milliseconds<'a>(&'a mut self, milliseconds: i32) -> &'a mut Self {
        self.milliseconds = milliseconds;
        return self;
    }

    pub fn enable_logs<'a>(&'a mut self, enable_logs: bool) -> &'a mut Self {
        self.enable_logs = enable_logs;
        return self;
    }

    pub fn get_audio_devices(&self) -> Result<Vec<String>, RecorderError> {
        return RecorderInner::get_audio_devices(self.library_path.clone());
    }

    pub fn init(&self) -> Result<Recorder, RecorderError> {
        let recorder_inner = RecorderInner::init(
            self.library_path.clone(),
            self.device_index,
            self.frame_length,
            self.milliseconds,
            self.enable_logs,
        );
        return recorder_inner.map(|inner| Recorder {
            inner: Arc::new(inner),
        });
    }
}

#[derive(Clone)]
pub struct Recorder {
    inner: Arc<RecorderInner>,
}

impl Recorder {
    pub fn start(&self) -> Result<(), RecorderError> {
        return self.inner.start();
    }

    pub fn stop(&self) -> Result<(), RecorderError> {
        return self.inner.stop();
    }

    pub fn read(&self, buffer: &mut [i16]) -> Result<(), RecorderError> {
        return self.inner.read(buffer);
    }

    pub fn frame_length(&self) -> usize {
        return self.inner.frame_length() as usize;
    }
}

macro_rules! load_library_fn {
    ($lib:ident, $function_name:literal) => {
        match $lib.get($function_name) {
            Ok(symbol) => symbol,
            Err(err) => {
                return Err(RecorderError::new(
                    RecorderErrorStatus::LibraryLoadError,
                    &format!(
                        "Failed to load function symbol from pvrecorder library: {}",
                        err
                    ),
                ))
            }
        };
    };
}

macro_rules! check_fn_call_status {
    ($status:ident, $function_name:literal) => {
        if $status != PvRecorderStatus::SUCCESS {
            return Err(RecorderError::new(
                RecorderErrorStatus::LibraryError($status),
                &format!(
                    "Function '{}' in the pvrecorder library failed",
                    $function_name
                ),
            ));
        }
    };
}

struct RecorderInnerVTable {
    pv_recorder_delete: RawSymbol<PvRecorderDeleteFn>,
    pv_recorder_start: RawSymbol<PvRecorderStartFn>,
    pv_recorder_stop: RawSymbol<PvRecorderStopFn>,
    pv_recorder_read: RawSymbol<PvRecorderReadFn>,
}

struct RecorderInner {
    cpvrecorder: *mut CPvRecorder,
    _lib: Library,
    frame_length: i32,
    vtable: RecorderInnerVTable,
}

impl RecorderInner {
    pub fn init<P: AsRef<Path>>(
        library_path: P,
        device_index: i32,
        frame_length: i32,
        milliseconds: i32,
        enable_logs: bool,
    ) -> Result<Self, RecorderError> {
        unsafe {
            if device_index < -1 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    &format!(
                        "device_index value {} should be greater than zero",
                        device_index
                    ),
                ));
            }

            if frame_length < 0 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    &format!(
                        "frame_length value {} should be greater than zero",
                        frame_length
                    ),
                ));
            }

            if milliseconds < 0 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    &format!(
                        "milliseconds value {} should be greater than zero",
                        milliseconds
                    ),
                ));
            }

            let lib = match Library::new(library_path.as_ref()) {
                Ok(symbol) => symbol,
                Err(err) => {
                    return Err(RecorderError::new(
                        RecorderErrorStatus::LibraryLoadError,
                        &format!("Failed to load pvrecorder dynamic library: {}", err),
                    ))
                }
            };

            let pv_recorder_init: Symbol<PvRecorderInitFn> =
                load_library_fn!(lib, b"pv_recorder_init");

            let mut cpvrecorder = std::ptr::null_mut();

            let status = pv_recorder_init(
                device_index,
                frame_length,
                milliseconds,
                enable_logs,
                addr_of_mut!(cpvrecorder),
            );
            if status != PvRecorderStatus::SUCCESS {
                return Err(RecorderError::new(
                    RecorderErrorStatus::LibraryLoadError,
                    "Failed to initialize the pvrecorder library",
                ));
            }

            let pv_recorder_delete: Symbol<PvRecorderDeleteFn> =
                load_library_fn!(lib, b"pv_recorder_delete");

            let pv_recorder_start: Symbol<PvRecorderStartFn> =
                load_library_fn!(lib, b"pv_recorder_start");

            let pv_recorder_stop: Symbol<PvRecorderStopFn> =
                load_library_fn!(lib, b"pv_recorder_stop");

            let pv_recorder_read: Symbol<PvRecorderReadFn> =
                load_library_fn!(lib, b"pv_recorder_read");

            // Using the raw symbols means we have to ensure that "lib" outlives these refrences
            let vtable = RecorderInnerVTable {
                pv_recorder_delete: pv_recorder_delete.into_raw(),
                pv_recorder_start: pv_recorder_start.into_raw(),
                pv_recorder_stop: pv_recorder_stop.into_raw(),
                pv_recorder_read: pv_recorder_read.into_raw(),
            };

            return Ok(Self {
                cpvrecorder,
                _lib: lib,
                frame_length,
                vtable,
            });
        }
    }

    fn start(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_start)(self.cpvrecorder) };
        check_fn_call_status!(status, "pv_recorder_start");

        return Ok(());
    }

    fn stop(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_stop)(self.cpvrecorder) };
        check_fn_call_status!(status, "pv_recorder_stop");

        return Ok(());
    }

    pub fn read(&self, pcm: &mut [i16]) -> Result<(), RecorderError> {
        if pcm.len() < self.frame_length as usize {
            return Err(RecorderError::new(
                RecorderErrorStatus::ArgumentError,
                &format!(
                    "PCM buffer needs to be at least the frame_size {}, currently {}",
                    self.frame_length,
                    pcm.len()
                ),
            ));
        }

        let status = unsafe { (self.vtable.pv_recorder_read)(self.cpvrecorder, pcm.as_mut_ptr()) };
        check_fn_call_status!(status, "pv_recorder_read");

        return Ok(());
    }

    pub fn get_audio_devices<P: AsRef<Path>>(
        library_path: P,
    ) -> Result<Vec<String>, RecorderError> {
        let mut devices = Vec::new();
        let mut count = 0;

        unsafe {
            let lib = match Library::new(library_path.as_ref()) {
                Ok(symbol) => symbol,
                Err(err) => {
                    return Err(RecorderError::new(
                        RecorderErrorStatus::LibraryLoadError,
                        &format!("Failed to load pvrecorder dynamic library: {}", err),
                    ))
                }
            };

            let pv_recorder_get_audio_devices: Symbol<PvRecorderGetAudioDevicesFn> =
                load_library_fn!(lib, b"pv_recorder_get_audio_devices");

            let pv_recorder_free_device_list: Symbol<PvRecorderFreeDeviceList> =
                load_library_fn!(lib, b"pv_recorder_free_device_list");

            let mut devices_ptr: *mut c_char = std::ptr::null_mut();
            let mut devices_ptr_ptr: *mut *mut c_char = addr_of_mut!(devices_ptr);

            let status =
                pv_recorder_get_audio_devices(addr_of_mut!(count), addr_of_mut!(devices_ptr_ptr));
            check_fn_call_status!(status, "pv_recorder_get_audio_devices");

            for i in 0..count as usize {
                let device = CStr::from_ptr(*devices_ptr_ptr.add(i));
                devices.push(String::from(device.to_str().map_err(|_| {
                    RecorderError::new(
                        RecorderErrorStatus::OtherError,
                        "Failed to convert device strings",
                    )
                })?))
            }

            pv_recorder_free_device_list(count, devices_ptr_ptr);
        }
        return Ok(devices);
    }

    pub fn frame_length(&self) -> i32 {
        return self.frame_length;
    }
}

unsafe impl Send for RecorderInner {}
unsafe impl Sync for RecorderInner {}

impl Drop for RecorderInner {
    fn drop(&mut self) {
        unsafe {
            (self.vtable.pv_recorder_delete)(self.cpvrecorder);
        }
    }
}
