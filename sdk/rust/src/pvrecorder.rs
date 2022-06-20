/*
    Copyright 2021-2022 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

use std::ffi::CStr;
use std::path::Path;
use std::ptr::addr_of_mut;
use std::sync::Arc;
use std::{cmp::PartialEq, path::PathBuf};

use libc::c_char;
use libloading::{Library, Symbol};

use crate::util::pv_library_path;

#[cfg(unix)]
use libloading::os::unix::Symbol as RawSymbol;
#[cfg(windows)]
use libloading::os::windows::Symbol as RawSymbol;

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
    log_silence: bool,
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

impl std::error::Error for RecorderError {}

const DEFAULT_DEVICE_INDEX: i32 = -1;
const DEFAULT_FRAME_LENGTH: i32 = 512;
const DEFAULT_MILLISECONDS: i32 = 1000;

pub struct RecorderBuilder {
    device_index: i32,
    frame_length: i32,
    buffer_size_msec: i32,
    log_overflow: bool,
    log_silence: bool,
    library_path: PathBuf,
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
            log_silence: true,
            library_path: pv_library_path(),
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

    pub fn log_silence(&mut self, log_silence: bool) -> &mut Self {
        self.log_silence = log_silence;
        self
    }

    pub fn library_path(&mut self, library_path: &Path) -> &mut Self {
        self.library_path = library_path.into();
        self
    }

    pub fn init(&self) -> Result<Recorder, RecorderError> {
        let recorder_inner = RecorderInner::init(
            self.device_index,
            self.frame_length,
            self.buffer_size_msec,
            self.log_overflow,
            self.log_silence,
            &self.library_path,
        );
        recorder_inner.map(|inner| Recorder {
            inner: Arc::new(inner),
        })
    }

    pub fn get_audio_devices(&self) -> Result<Vec<String>, RecorderError> {
        RecorderInner::get_audio_devices(&self.library_path)
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
}

unsafe fn load_library_fn<T>(
    library: &Library,
    function_name: &[u8],
) -> Result<RawSymbol<T>, RecorderError> {
    library
        .get(function_name)
        .map(|s: Symbol<T>| s.into_raw())
        .map_err(|err| {
            RecorderError::new(
                RecorderErrorStatus::LibraryLoadError,
                format!(
                    "Failed to load function symbol from pvrecorder library: {}",
                    err
                ),
            )
        })
}

fn check_fn_call_status(
    status: PvRecorderStatus,
    function_name: &str,
) -> Result<(), RecorderError> {
    match status {
        PvRecorderStatus::SUCCESS => Ok(()),
        _ => Err(RecorderError::new(
            RecorderErrorStatus::LibraryError(status),
            format!(
                "Function '{}' in the pvrecorder library failed",
                function_name
            ),
        )),
    }
}

struct RecorderInnerVTable {
    pv_recorder_delete: RawSymbol<PvRecorderDeleteFn>,
    pv_recorder_start: RawSymbol<PvRecorderStartFn>,
    pv_recorder_stop: RawSymbol<PvRecorderStopFn>,
    pv_recorder_read: RawSymbol<PvRecorderReadFn>,
    pv_recorder_get_audio_devices: RawSymbol<PvRecorderGetAudioDevicesFn>,
    pv_recorder_free_device_list: RawSymbol<PvRecorderFreeDeviceList>,

    _lib_guard: Library,
}

impl RecorderInnerVTable {
    pub fn new(lib: Library) -> Result<Self, RecorderError> {
        // SAFETY: the library will be hold by this struct and therefore the symbols can't outlive the library
        unsafe {
            Ok(Self {
                pv_recorder_delete: load_library_fn(&lib, b"pv_recorder_delete")?,
                pv_recorder_start: load_library_fn(&lib, b"pv_recorder_start")?,
                pv_recorder_stop: load_library_fn(&lib, b"pv_recorder_stop")?,
                pv_recorder_read: load_library_fn(&lib, b"pv_recorder_read")?,
                pv_recorder_get_audio_devices: load_library_fn(
                    &lib,
                    b"pv_recorder_get_audio_devices",
                )?,
                pv_recorder_free_device_list: load_library_fn(
                    &lib,
                    b"pv_recorder_free_device_list",
                )?,

                _lib_guard: lib,
            })
        }
    }
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
        log_silence: bool,
        library_path: &Path,
    ) -> Result<Self, RecorderError> {
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

        let lib = unsafe { Library::new(library_path) }.map_err(|err| {
            RecorderError::new(
                RecorderErrorStatus::LibraryLoadError,
                format!("Failed to load pvrecorder dynamic library: {}", err),
            )
        })?;

        let mut cpvrecorder = std::ptr::null_mut();

        // SAFETY: loading the library is safe, because we still have the lib handle
        unsafe {
            let pv_recorder_init = load_library_fn::<PvRecorderInitFn>(&lib, b"pv_recorder_init")?;

            let status = pv_recorder_init(
                device_index,
                frame_length,
                buffer_size_msec,
                log_overflow,
                log_silence,
                addr_of_mut!(cpvrecorder),
            );
            check_fn_call_status(status, "pv_recorder_init")?;
        }

        Ok(Self {
            cpvrecorder,
            frame_length,
            vtable: RecorderInnerVTable::new(lib)?,
        })
    }

    fn start(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_start)(self.cpvrecorder) };
        check_fn_call_status(status, "pv_recorder_start")
    }

    fn stop(&self) -> Result<(), RecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_stop)(self.cpvrecorder) };
        check_fn_call_status(status, "pv_recorder_stop")
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
        check_fn_call_status(status, "pv_recorder_read")
    }

    pub fn get_audio_devices(library_path: &Path) -> Result<Vec<String>, RecorderError> {
        let lib = unsafe { Library::new(library_path) }.map_err(|err| {
            RecorderError::new(
                RecorderErrorStatus::LibraryLoadError,
                format!("Failed to load pvrecorder dynamic library: {}", err),
            )
        })?;

        let vtable = RecorderInnerVTable::new(lib)?;

        let mut devices = Vec::new();
        let mut count = 0;

        unsafe {
            let mut devices_ptr: *mut c_char = std::ptr::null_mut();
            let mut devices_ptr_ptr: *mut *mut c_char = addr_of_mut!(devices_ptr);

            let status = (vtable.pv_recorder_get_audio_devices)(
                addr_of_mut!(count),
                addr_of_mut!(devices_ptr_ptr),
            );
            check_fn_call_status(status, "pv_recorder_get_audio_devices")?;

            for i in 0..count as usize {
                let device = CStr::from_ptr(*devices_ptr_ptr.add(i));
                devices.push(String::from(device.to_str().map_err(|_| {
                    RecorderError::new(
                        RecorderErrorStatus::OtherError,
                        "Failed to convert device strings",
                    )
                })?));
            }

            (vtable.pv_recorder_free_device_list)(count, devices_ptr_ptr);
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
