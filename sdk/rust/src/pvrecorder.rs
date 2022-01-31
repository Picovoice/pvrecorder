/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

use lazy_static::lazy_static;
use libc::c_char;
use libloading::{Library, Symbol};
use std::cmp::PartialEq;
use std::ffi::CStr;
use std::ptr::addr_of_mut;
use std::sync::Arc;

lazy_static! {
    static ref PV_RECORDER_LIB: Result<Library, RecorderError> = {
        unsafe {
            match Library::new(pv_library_path()) {
                Ok(symbol) => Ok(symbol),
                Err(err) => Err(RecorderError::new(
                    RecorderErrorStatus::LibraryLoadError,
                    format!("Failed to load pvrecorder dynamic library: {}", err),
                )),
            }
        }
    };
}

use crate::util::pv_library_path;

#[repr(C)]
struct CPvRecorder {}

#[repr(C)]
#[derive(PartialEq, Clone, Debug)]
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
    buffer_size_msec: i32,
    log_overflow: bool,
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

#[derive(Clone, Debug)]
pub enum RecorderErrorStatus {
    LibraryError(PvRecorderStatus),
    LibraryLoadError,
    ArgumentError,
    OtherError,
}

#[derive(Clone, Debug)]
pub struct RecorderError {
    status: RecorderErrorStatus,
    message: String,
}

impl RecorderError {
    pub fn new(status: RecorderErrorStatus, message: impl Into<String>) -> Self {
        Self {
            status,
            message: message.into(),
        }
    }
}

impl std::fmt::Display for RecorderError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}: {:?}", self.message, self.status)
    }
}

const DEFAULT_DEVICE_INDEX: i32 = -1;
const DEFAULT_FRAME_LENGTH: i32 = 512;
const DEFAULT_MILLISECONDS: i32 = 1000;

pub struct RecorderBuilder {
    device_index: i32,
    frame_length: i32,
    buffer_size_msec: i32,
    log_overflow: bool,
}

impl Default for RecorderBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl RecorderBuilder {
    pub fn new() -> Self {
        Self {
            device_index: DEFAULT_DEVICE_INDEX,
            frame_length: DEFAULT_FRAME_LENGTH,
            buffer_size_msec: DEFAULT_MILLISECONDS,
            log_overflow: false,
        }
    }

    pub fn device_index(&mut self, device_index: i32) -> &mut Self {
        self.device_index = device_index;
        self
    }

    pub fn frame_length(&mut self, frame_length: i32) -> &mut Self {
        self.frame_length = frame_length;
        self
    }

    pub fn buffer_size_msec(&mut self, buffer_size_msec: i32) -> &mut Self {
        self.buffer_size_msec = buffer_size_msec;
        self
    }

    pub fn log_overflow(&mut self, log_overflow: bool) -> &mut Self {
        self.log_overflow = log_overflow;
        self
    }

    pub fn init(&self) -> Result<Recorder, RecorderError> {
        let recorder_inner = RecorderInner::init(
            self.device_index,
            self.frame_length,
            self.buffer_size_msec,
            self.log_overflow,
        );
        recorder_inner.map(|inner| Recorder {
            inner: Arc::new(inner),
        })
    }
}

#[derive(Clone)]
pub struct Recorder {
    inner: Arc<RecorderInner>,
}

impl Recorder {
    pub fn start(&self) -> Result<(), RecorderError> {
        self.inner.start()
    }

    pub fn stop(&self) -> Result<(), RecorderError> {
        self.inner.stop()
    }

    pub fn read(&self, buffer: &mut [i16]) -> Result<(), RecorderError> {
        self.inner.read(buffer)
    }

    pub fn frame_length(&self) -> usize {
        self.inner.frame_length() as usize
    }

    pub fn get_audio_devices() -> Result<Vec<String>, RecorderError> {
        RecorderInner::get_audio_devices()
    }
}

fn load_library_fn<T>(function_name: &[u8]) -> Result<Symbol<T>, RecorderError> {
    match &*PV_RECORDER_LIB {
        Ok(lib) => unsafe {
            lib.get(function_name).map_err(|err| {
                RecorderError::new(
                    RecorderErrorStatus::LibraryLoadError,
                    format!(
                        "Failed to load function symbol from pvrecorder library: {}",
                        err
                    ),
                )
            })
        },
        Err(err) => Err((*err).clone()),
    }
}

macro_rules! check_fn_call_status {
    ($status:ident, $function_name:literal) => {
        if $status != PvRecorderStatus::SUCCESS {
            return Err(RecorderError::new(
                RecorderErrorStatus::LibraryError($status),
                format!(
                    "Function '{}' in the pvrecorder library failed",
                    $function_name
                ),
            ));
        }
    };
}

struct RecorderInnerVTable {
    pv_recorder_delete: Symbol<'static, PvRecorderDeleteFn>,
    pv_recorder_start: Symbol<'static, PvRecorderStartFn>,
    pv_recorder_stop: Symbol<'static, PvRecorderStopFn>,
    pv_recorder_read: Symbol<'static, PvRecorderReadFn>,
}

struct RecorderInner {
    cpvrecorder: *mut CPvRecorder,
    frame_length: i32,
    vtable: RecorderInnerVTable,
}

impl RecorderInner {
    pub fn init(
        device_index: i32,
        frame_length: i32,
        buffer_size_msec: i32,
        log_overflow: bool,
    ) -> Result<Self, RecorderError> {
        unsafe {
            if device_index < -1 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    format!(
                        "device_index value {} should be greater than zero",
                        device_index
                    ),
                ));
            }

            if frame_length < 0 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    format!(
                        "frame_length value {} should be greater than zero",
                        frame_length
                    ),
                ));
            }

            if buffer_size_msec < 0 {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    format!(
                        "buffer_size_msec value {} should be greater than zero",
                        buffer_size_msec
                    ),
                ));
            }

            if buffer_size_msec < frame_length {
                return Err(RecorderError::new(
                    RecorderErrorStatus::ArgumentError,
                    format!(
                        "buffer_size_msec value {} should be greater than the frame length {}",
                        buffer_size_msec, frame_length
                    ),
                ));
            }

            let pv_recorder_init: Symbol<PvRecorderInitFn> = load_library_fn(b"pv_recorder_init")?;
            let mut cpvrecorder = std::ptr::null_mut();

            let status = pv_recorder_init(
                device_index,
                frame_length,
                buffer_size_msec,
                log_overflow,
                addr_of_mut!(cpvrecorder),
            );
            if status != PvRecorderStatus::SUCCESS {
                return Err(RecorderError::new(
                    RecorderErrorStatus::LibraryLoadError,
                    format!("Failed to initialize the pvrecorder library ({:?})", status),
                ));
            }

            let pv_recorder_delete: Symbol<PvRecorderDeleteFn> =
                load_library_fn(b"pv_recorder_delete")?;
            let pv_recorder_start: Symbol<PvRecorderStartFn> =
                load_library_fn(b"pv_recorder_start")?;
            let pv_recorder_stop: Symbol<PvRecorderStopFn> = load_library_fn(b"pv_recorder_stop")?;
            let pv_recorder_read: Symbol<PvRecorderReadFn> = load_library_fn(b"pv_recorder_read")?;

            let vtable = RecorderInnerVTable {
                pv_recorder_delete,
                pv_recorder_start,
                pv_recorder_stop,
                pv_recorder_read,
            };

            Ok(Self {
                cpvrecorder,
                frame_length,
                vtable,
            })
        }
    }

    fn start(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_start)(self.cpvrecorder) };
        check_fn_call_status!(status, "pv_recorder_start");

        Ok(())
    }

    fn stop(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_stop)(self.cpvrecorder) };
        check_fn_call_status!(status, "pv_recorder_stop");

        Ok(())
    }

    pub fn read(&self, pcm: &mut [i16]) -> Result<(), RecorderError> {
        if pcm.len() < self.frame_length as usize {
            return Err(RecorderError::new(
                RecorderErrorStatus::ArgumentError,
                format!(
                    "PCM buffer needs to be at least the frame_size {}, currently {}",
                    self.frame_length,
                    pcm.len()
                ),
            ));
        }

        let status = unsafe { (self.vtable.pv_recorder_read)(self.cpvrecorder, pcm.as_mut_ptr()) };
        check_fn_call_status!(status, "pv_recorder_read");

        Ok(())
    }

    pub fn get_audio_devices() -> Result<Vec<String>, RecorderError> {
        let mut devices = Vec::new();
        let mut count = 0;

        unsafe {
            let pv_recorder_get_audio_devices: Symbol<PvRecorderGetAudioDevicesFn> =
                load_library_fn(b"pv_recorder_get_audio_devices")?;
            let pv_recorder_free_device_list: Symbol<PvRecorderFreeDeviceList> =
                load_library_fn(b"pv_recorder_free_device_list")?;

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
                })?));
            }

            pv_recorder_free_device_list(count, devices_ptr_ptr);
        }
        Ok(devices)
    }

    pub fn frame_length(&self) -> i32 {
        self.frame_length
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
