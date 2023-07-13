/*
    Copyright 2021-2023 Picovoice Inc.

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
    RUNTIME_ERROR = 8,
}

type PvRecorderInitFn = unsafe extern "C" fn(
    frame_length: i32,
    device_index: i32,
    buffered_frames_count: i32,
    object: *mut *mut CPvRecorder,
) -> PvRecorderStatus;
type PvRecorderDeleteFn = unsafe extern "C" fn(object: *mut CPvRecorder);
type PvRecorderStartFn = unsafe extern "C" fn(object: *mut CPvRecorder) -> PvRecorderStatus;
type PvRecorderStopFn = unsafe extern "C" fn(object: *mut CPvRecorder) -> PvRecorderStatus;
type PvRecorderReadFn =
    unsafe extern "C" fn(object: *mut CPvRecorder, pcm: *mut i16) -> PvRecorderStatus;
type PvRecorderSetDebugLoggingFn =
    unsafe extern "C" fn(object: *mut CPvRecorder, is_debug_logging: bool);
type PvRecorderGetIsRecordingFn = unsafe extern "C" fn(object: *mut CPvRecorder) -> bool;
type PvRecorderGetSelectedDeviceFn =
    unsafe extern "C" fn(object: *mut CPvRecorder) -> *const c_char;
type PvRecorderGetAvailableDevicesFn = unsafe extern "C" fn(
    device_list_length: *mut i32,
    device_list: *mut *mut *mut c_char,
) -> PvRecorderStatus;
type PvRecorderFreeAvailableDevicesList =
    unsafe extern "C" fn(device_list_length: i32, device_list: *mut *mut c_char);

type PvRecorderSampleRate = unsafe extern "C" fn() -> i32;
type PvRecorderVersion = unsafe extern "C" fn() -> *const c_char;

#[derive(Clone, Debug)]
pub enum PvRecorderErrorStatus {
    LibraryError(PvRecorderStatus),
    LibraryLoadError,
    ArgumentError,
    OtherError,
}

#[derive(Clone, Debug)]
pub struct PvRecorderError {
    status: PvRecorderErrorStatus,
    message: String,
}

impl PvRecorderError {
    pub fn new(status: PvRecorderErrorStatus, message: impl Into<String>) -> Self {
        Self {
            status,
            message: message.into(),
        }
    }
}

impl std::fmt::Display for PvRecorderError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}: {:?}", self.message, self.status)
    }
}

impl std::error::Error for PvRecorderError {}

const DEFAULT_DEVICE_INDEX: i32 = -1;
const DEFAULT_FRAME_LENGTH: i32 = 512;
const DEFAULT_BUFFERED_FRAMES_COUNT: i32 = 50;

pub struct PvRecorderBuilder {
    frame_length: i32,
    device_index: i32,
    buffered_frames_count: i32,
    library_path: PathBuf,
}

impl Default for PvRecorderBuilder {
    fn default() -> Self {
        Self::new(DEFAULT_FRAME_LENGTH)
    }
}

impl PvRecorderBuilder {
    pub fn new(frame_length: i32) -> Self {
        Self {
            frame_length,
            device_index: DEFAULT_DEVICE_INDEX,
            buffered_frames_count: DEFAULT_BUFFERED_FRAMES_COUNT,
            library_path: pv_library_path(),
        }
    }

    pub fn frame_length(&mut self, frame_length: i32) -> &mut Self {
        self.frame_length = frame_length;
        self
    }

    pub fn device_index(&mut self, device_index: i32) -> &mut Self {
        self.device_index = device_index;
        self
    }

    pub fn buffered_frames_count(&mut self, buffered_frames_count: i32) -> &mut Self {
        self.buffered_frames_count = buffered_frames_count;
        self
    }

    pub fn library_path(&mut self, library_path: &Path) -> &mut Self {
        self.library_path = library_path.into();
        self
    }

    pub fn init(&self) -> Result<PvRecorder, PvRecorderError> {
        if self.frame_length <= 0 {
            return Err(PvRecorderError::new(
                PvRecorderErrorStatus::ArgumentError,
                format!(
                    "frame_length needs to be greater than or equal to 0, got: {}",
                    self.frame_length
                ),
            ));
        }

        let recorder_inner = PvRecorderInner::init(
            self.frame_length,
            self.device_index,
            self.buffered_frames_count,
            &self.library_path,
        );
        recorder_inner.map(|inner| PvRecorder {
            inner: Arc::new(inner),
        })
    }

    pub fn get_available_devices(&self) -> Result<Vec<String>, PvRecorderError> {
        PvRecorderInner::get_available_devices(&self.library_path)
    }
}

#[derive(Clone)]
pub struct PvRecorder {
    inner: Arc<PvRecorderInner>,
}

impl PvRecorder {
    pub fn start(&self) -> Result<(), PvRecorderError> {
        self.inner.start()
    }

    pub fn stop(&self) -> Result<(), PvRecorderError> {
        self.inner.stop()
    }

    pub fn read(&self) -> Result<Vec<i16>, PvRecorderError> {
        self.inner.read()
    }

    pub fn set_debug_logging(&self, is_debug_logging_enabled: bool) {
        self.inner.set_debug_logging(is_debug_logging_enabled)
    }

    pub fn frame_length(&self) -> usize {
        self.inner.frame_length() as usize
    }

    pub fn is_recording(&self) -> bool {
        self.inner.is_recording()
    }

    pub fn sample_rate(&self) -> usize {
        self.inner.sample_rate() as usize
    }

    pub fn selected_device(&self) -> String {
        self.inner.selected_device()
    }

    pub fn version(&self) -> String {
        self.inner.version()
    }
}

unsafe fn load_library_fn<T>(
    library: &Library,
    function_name: &[u8],
) -> Result<RawSymbol<T>, PvRecorderError> {
    library
        .get(function_name)
        .map(|s: Symbol<T>| s.into_raw())
        .map_err(|err| {
            PvRecorderError::new(
                PvRecorderErrorStatus::LibraryLoadError,
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
) -> Result<(), PvRecorderError> {
    match status {
        PvRecorderStatus::SUCCESS => Ok(()),
        _ => Err(PvRecorderError::new(
            PvRecorderErrorStatus::LibraryError(status),
            format!(
                "Function '{}' in the pvrecorder library failed",
                function_name
            ),
        )),
    }
}

struct PvRecorderInnerVTable {
    pv_recorder_init: RawSymbol<PvRecorderInitFn>,
    pv_recorder_delete: RawSymbol<PvRecorderDeleteFn>,
    pv_recorder_start: RawSymbol<PvRecorderStartFn>,
    pv_recorder_stop: RawSymbol<PvRecorderStopFn>,
    pv_recorder_read: RawSymbol<PvRecorderReadFn>,
    pv_recorder_set_debug_logging: RawSymbol<PvRecorderSetDebugLoggingFn>,
    pv_recorder_get_is_recording: RawSymbol<PvRecorderGetIsRecordingFn>,
    pv_recorder_get_selected_device: RawSymbol<PvRecorderGetSelectedDeviceFn>,
    pv_recorder_get_available_devices: RawSymbol<PvRecorderGetAvailableDevicesFn>,
    pv_recorder_free_available_devices: RawSymbol<PvRecorderFreeAvailableDevicesList>,
    pv_recorder_sample_rate: RawSymbol<PvRecorderSampleRate>,
    pv_recorder_version: RawSymbol<PvRecorderVersion>,

    _lib_guard: Library,
}

impl PvRecorderInnerVTable {
    pub fn new(lib: Library) -> Result<Self, PvRecorderError> {
        // SAFETY: the library will be hold by this struct and therefore the symbols can't outlive the library
        unsafe {
            Ok(Self {
                pv_recorder_init: load_library_fn(&lib, b"pv_recorder_init")?,
                pv_recorder_delete: load_library_fn(&lib, b"pv_recorder_delete")?,
                pv_recorder_start: load_library_fn(&lib, b"pv_recorder_start")?,
                pv_recorder_stop: load_library_fn(&lib, b"pv_recorder_stop")?,
                pv_recorder_read: load_library_fn(&lib, b"pv_recorder_read")?,
                pv_recorder_set_debug_logging: load_library_fn(
                    &lib,
                    b"pv_recorder_set_debug_logging",
                )?,
                pv_recorder_get_is_recording: load_library_fn(
                    &lib,
                    b"pv_recorder_get_is_recording",
                )?,
                pv_recorder_get_selected_device: load_library_fn(
                    &lib,
                    b"pv_recorder_get_selected_device",
                )?,
                pv_recorder_get_available_devices: load_library_fn(
                    &lib,
                    b"pv_recorder_get_available_devices",
                )?,
                pv_recorder_free_available_devices: load_library_fn(
                    &lib,
                    b"pv_recorder_free_available_devices",
                )?,
                pv_recorder_sample_rate: load_library_fn(&lib, b"pv_recorder_sample_rate")?,
                pv_recorder_version: load_library_fn(&lib, b"pv_recorder_version")?,

                _lib_guard: lib,
            })
        }
    }
}

struct PvRecorderInner {
    cpvrecorder: *mut CPvRecorder,
    frame_length: i32,
    sample_rate: i32,
    selected_device: String,
    version: String,
    vtable: PvRecorderInnerVTable,
}

impl PvRecorderInner {
    pub fn init(
        frame_length: i32,
        device_index: i32,
        buffered_frames_count: i32,
        library_path: &Path,
    ) -> Result<Self, PvRecorderError> {
        if frame_length <= 0 {
            return Err(PvRecorderError::new(
                PvRecorderErrorStatus::ArgumentError,
                format!(
                    "frame_length value `{}` should be greater than zero",
                    frame_length
                ),
            ));
        }

        if device_index < -1 {
            return Err(PvRecorderError::new(
                PvRecorderErrorStatus::ArgumentError,
                format!(
                    "device_index value `{}` should be greater than or equal to -1",
                    device_index
                ),
            ));
        }

        if buffered_frames_count <= 0 {
            return Err(PvRecorderError::new(
                PvRecorderErrorStatus::ArgumentError,
                format!(
                    "buffered_frames_count value `{}` should be greater than zero",
                    buffered_frames_count
                ),
            ));
        }

        let lib = unsafe { Library::new(library_path) }.map_err(|err| {
            PvRecorderError::new(
                PvRecorderErrorStatus::LibraryLoadError,
                format!("Failed to load pvrecorder dynamic library: {}", err),
            )
        })?;
        let vtable = PvRecorderInnerVTable::new(lib)?;

        let mut cpvrecorder = std::ptr::null_mut();

        unsafe {
            let status = (vtable.pv_recorder_init)(
                frame_length,
                device_index,
                buffered_frames_count,
                addr_of_mut!(cpvrecorder),
            );
            check_fn_call_status(status, "pv_recorder_init")?;
        }

        let selected_device = unsafe {
            let selected_device_c = (vtable.pv_recorder_get_selected_device)(cpvrecorder);
            String::from(CStr::from_ptr(selected_device_c).to_str().map_err(|_| {
                PvRecorderError::new(
                    PvRecorderErrorStatus::OtherError,
                    "Failed to convert selected device string",
                )
            })?)
        };

        let sample_rate = unsafe { (vtable.pv_recorder_sample_rate)() };

        let version = unsafe {
            let version_c = (vtable.pv_recorder_version)();
            String::from(CStr::from_ptr(version_c).to_str().map_err(|_| {
                PvRecorderError::new(
                    PvRecorderErrorStatus::OtherError,
                    "Failed to convert version string",
                )
            })?)
        };

        Ok(Self {
            cpvrecorder,
            frame_length,
            sample_rate,
            selected_device,
            version,
            vtable,
        })
    }

    fn start(&self) -> Result<(), PvRecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_start)(self.cpvrecorder) };
        check_fn_call_status(status, "pv_recorder_start")
    }

    fn stop(&self) -> Result<(), PvRecorderError> {
        let status = unsafe { (self.vtable.pv_recorder_stop)(self.cpvrecorder) };
        check_fn_call_status(status, "pv_recorder_stop")
    }

    fn read(&self) -> Result<Vec<i16>, PvRecorderError> {
        let mut frame = vec![0; self.frame_length() as usize];
        let status =
            unsafe { (self.vtable.pv_recorder_read)(self.cpvrecorder, frame.as_mut_ptr()) };
        check_fn_call_status(status, "pv_recorder_read")?;
        Ok(frame)
    }

    fn set_debug_logging(&self, is_debug_logging_enabled: bool) {
        unsafe {
            (self.vtable.pv_recorder_set_debug_logging)(self.cpvrecorder, is_debug_logging_enabled)
        };
    }

    fn frame_length(&self) -> i32 {
        self.frame_length
    }

    fn is_recording(&self) -> bool {
        unsafe { (self.vtable.pv_recorder_get_is_recording)(self.cpvrecorder) }
    }

    fn sample_rate(&self) -> i32 {
        self.sample_rate
    }

    fn selected_device(&self) -> String {
        self.selected_device.clone()
    }

    fn version(&self) -> String {
        self.version.clone()
    }

    pub fn get_available_devices<P: AsRef<Path>>(
        library_path: P,
    ) -> Result<Vec<String>, PvRecorderError> {
        let lib = unsafe { Library::new(library_path.as_ref()) }.map_err(|err| {
            PvRecorderError::new(
                PvRecorderErrorStatus::LibraryLoadError,
                format!("Failed to load pvrecorder dynamic library: {}", err),
            )
        })?;

        let vtable = PvRecorderInnerVTable::new(lib)?;

        let mut device_list = Vec::new();
        let mut device_list_length = 0;

        unsafe {
            let mut device_list_ptr: *mut c_char = std::ptr::null_mut();
            let mut device_list_ptr_ptr: *mut *mut c_char = addr_of_mut!(device_list_ptr);

            let status = (vtable.pv_recorder_get_available_devices)(
                addr_of_mut!(device_list_length),
                addr_of_mut!(device_list_ptr_ptr),
            );
            check_fn_call_status(status, "pv_recorder_get_available_devices")?;

            for i in 0..device_list_length as usize {
                let device = CStr::from_ptr(*device_list_ptr_ptr.add(i));
                device_list.push(String::from(device.to_str().map_err(|_| {
                    PvRecorderError::new(
                        PvRecorderErrorStatus::OtherError,
                        "Failed to convert device strings",
                    )
                })?));
            }

            (vtable.pv_recorder_free_available_devices)(device_list_length, device_list_ptr_ptr);
        }
        Ok(device_list)
    }
}

unsafe impl Send for PvRecorderInner {}
unsafe impl Sync for PvRecorderInner {}

impl Drop for PvRecorderInner {
    fn drop(&mut self) {
        unsafe {
            (self.vtable.pv_recorder_delete)(self.cpvrecorder);
        }
    }
}
