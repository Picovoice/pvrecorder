/*
    Copyright 2021-2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Pv
{
    /// <summary>
    /// Status codes return by PvRecorder Library.
    /// </summary>
    public enum PvRecorderStatus
    {
        SUCCESS = 0,
        OUT_OF_MEMORY = 1,
        INVALID_ARGUMENT = 2,
        INVALID_STATE = 3,
        BACKEND_ERROR = 4,
        DEVICE_ALREADY_INITIALIZED = 5,
        DEVICE_NOT_INITIALIZED = 6,
        IO_ERROR = 7,
        RUNTIME_ERROR = 8
    }

    /// <summary>
    /// PvRecorder is a cross-platform audio recorder library for .NET that is designed for real-time audio processing.
    /// </summary>
    public class PvRecorder : IDisposable
    {
        private const string LIBRARY = "libpv_recorder";
        private IntPtr _libraryPointer = IntPtr.Zero;

        static PvRecorder()
        {

#if NETCOREAPP3_0_OR_GREATER

            NativeLibrary.SetDllImportResolver(typeof(PvRecorder).Assembly, ImportResolver);

#endif

        }

#if NETCOREAPP3_0_OR_GREATER

        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {

#pragma warning disable IDE0058
#pragma warning disable IDE0059

            IntPtr libHandle = IntPtr.Zero;
            NativeLibrary.TryLoad(GetLibraryPath(), out libHandle);
            return libHandle;
        }

#pragma warning restore IDE0059
#pragma warning restore IDE0058

#endif

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern PvRecorderStatus pv_recorder_init(int frameLength, int deviceIndex, int bufferedFramesCount, out IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern void pv_recorder_delete(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern PvRecorderStatus pv_recorder_start(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern PvRecorderStatus pv_recorder_stop(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern PvRecorderStatus pv_recorder_read(IntPtr handle, short[] frame);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr pv_recorder_set_debug_logging(IntPtr handle, bool isDebugLoggingEnabled);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern char pv_recorder_get_is_recording(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr pv_recorder_get_selected_device(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern PvRecorderStatus pv_recorder_get_available_devices(out int deviceListLength, out IntPtr deviceList);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern void pv_recorder_free_available_devices(int deviceListLength, IntPtr deviceList);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern int pv_recorder_sample_rate();

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr pv_recorder_version();

        /// <summary>
        /// Factory method for creating instances of PvRecorder.
        /// </summary>
        /// <param name="frameLength">
        /// Length of the audio frame to receive with each call to read.
        /// </param>
        /// <param name="deviceIndex">
        /// The index of the audio device to capture audio from. A value of (-1) will use the default audio.
        /// </param>
        /// <param name="bufferedFramesCount">
        /// The number of audio frames buffered internally for reading - i.e. internal circular buffer
        /// will be of size `frame_length` * `buffered_frames_count`. If this value is too low, buffer overflows could occur
        /// and audio frames could be dropped. A higher value will increase memory usage.
        /// </param>
        /// <returns>An instance of PvRecorder.</returns>
        public static PvRecorder Create(int frameLength, int deviceIndex = -1, int bufferedFramesCount = 50)
        {
            return new PvRecorder(frameLength, deviceIndex, bufferedFramesCount);
        }

        /// <summary>
        /// Constructor for PvRecorder.
        /// </summary>
        /// <param name="deviceIndex">
        /// The index of the audio device to capture audio from. A value of (-1) will use the default audio.
        /// </param>
        /// <param name="frameLength">
        /// Length of the audio frames to receive at each read call.
        /// </param>
        /// <param name="bufferedFramesCount">
        /// The number of audio frames buffered internally for reading - i.e. internal circular buffer
        /// will be of size `frame_length` * `buffered_frames_count`. If this value is too low, buffer overflows could occur
        /// and audio frames could be dropped. A higher value will increase memory usage.
        /// </param>
        private PvRecorder(int frameLength, int deviceIndex, int bufferedFramesCount)
        {
            if (frameLength <= 0)
            {
                throw new PvRecorderInvalidArgumentException($"Frame length of {frameLength} is invalid - must be greater than 0.");
            }

            if (deviceIndex < -1)
            {
                throw new PvRecorderInvalidArgumentException($"Device index of {deviceIndex} is invalid - must be greater than -1.");
            }

            if (bufferedFramesCount <= 0)
            {
                throw new PvRecorderInvalidArgumentException($"Buffered frames count of {bufferedFramesCount} is invalid - must be greater than 0.");
            }

            PvRecorderStatus status = pv_recorder_init(frameLength, deviceIndex, bufferedFramesCount, out _libraryPointer);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }

            FrameLength = frameLength;
            SampleRate = pv_recorder_sample_rate();
            SelectedDevice = Marshal.PtrToStringAnsi(pv_recorder_get_selected_device(_libraryPointer));
            Version = Marshal.PtrToStringAnsi(pv_recorder_version());
        }

        /// <summary>
        /// Starts recording audio. Should be called before making any calls to `Read()` or `Stop()`.
        /// </summary>
        public void Start()
        {
            PvRecorderStatus status = pv_recorder_start(_libraryPointer);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }
        }

        /// <summary>
        /// Stops recording audio. Should only be called after a successful call to `Start()`.
        /// </summary>
        public void Stop()
        {
            PvRecorderStatus status = pv_recorder_stop(_libraryPointer);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }
        }

        /// <summary>
        /// Synchronously reads a frame of audio samples. Call between `Start()` and `Stop()`.
        /// </summary>
        /// <returns>An array of audio samples with length of `frameLength` that was provided upon initialization.</returns>
        public short[] Read()
        {
            short[] frame = new short[FrameLength];
            PvRecorderStatus status = pv_recorder_read(_libraryPointer, frame);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }
            return frame;
        }

        /// <summary>
        /// Enable or disable debug logging. Debug logs will indicate when there are overflows
        /// in the internal frame buffer and when an audio source is generating frames of silence.
        /// </summary>
        /// <param name="isDebugLoggingEnabled">Boolean indicating whether the debug logging is enabled or disabled.</param>
        public void SetDebugLogging(bool isDebugLoggingEnabled)
        {
            pv_recorder_set_debug_logging(_libraryPointer, isDebugLoggingEnabled);
        }

        /// <summary>
        /// Gets the length of frame returned by the recorder.
        /// </summary>
        public int FrameLength
        {
            get; private set;
        }

        /// <summary>
        /// Gets whether the recorder is currently capturing audio or not.
        /// </summary>
        public bool IsRecording
        {
            get
            {
                return pv_recorder_get_is_recording(_libraryPointer) != 0;
            }
        }

        /// <summary>
        /// Gets the recording sample rate.
        /// </summary>
        public int SampleRate
        {
            get; private set;
        }


        /// <summary>
        /// Gets the current selected audio device.
        /// </summary>
        public string SelectedDevice
        {
            get; private set;
        }

        /// <summary>
        /// Gets the current version of the library.
        /// </summary>
        public string Version
        {
            get; private set;
        }

        /// <summary>
        /// Gets a list of the available audio input devices on the current system.
        /// </summary>
        /// <returns>An array of strings containing the names of the audio devices.</returns>
        public static string[] GetAvailableDevices()
        {
            int deviceListLength;
            IntPtr deviceList;

            PvRecorderStatus status = pv_recorder_get_available_devices(out deviceListLength, out deviceList);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }

            int elementSize = Marshal.SizeOf(typeof(IntPtr));
            string[] deviceNames = new string[deviceListLength];
            for (int i = 0; i < deviceListLength; i++)
            {
                deviceNames[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(deviceList, i * elementSize));
            }

            pv_recorder_free_available_devices(deviceListLength, deviceList);

            return deviceNames;
        }

        /// <summary>
        /// Converts status codes to PvRecorderExceptions.
        /// </summary>
        /// <param name="status">Status code.</param>
        /// <returns>PvRecorderExceptions</returns>
        private static PvRecorderException PvRecorderStatusToException(PvRecorderStatus status)
        {
            switch (status)
            {
                case PvRecorderStatus.OUT_OF_MEMORY:
                    return new PvRecorderMemoryException();
                case PvRecorderStatus.INVALID_ARGUMENT:
                    return new PvRecorderInvalidArgumentException();
                case PvRecorderStatus.INVALID_STATE:
                    return new PvRecorderInvalidStateException("PvRecorder failed with invalid state.");
                case PvRecorderStatus.BACKEND_ERROR:
                    return new PvRecorderBackendException("PvRecorder audio backend error.");
                case PvRecorderStatus.DEVICE_ALREADY_INITIALIZED:
                    return new PvRecorderDeviceAlreadyInitializedException("PvRecorder audio device already initialized.");
                case PvRecorderStatus.DEVICE_NOT_INITIALIZED:
                    return new PvRecorderDeviceNotInitializedException("PvRecorder audio device not initialized.");
                case PvRecorderStatus.IO_ERROR:
                    return new PvRecorderIOException();
                case PvRecorderStatus.RUNTIME_ERROR:
                    return new PvRecorderRuntimeException("PvRecorder runtime error.");
                default:
                    return new PvRecorderException("Unknown status returned from PvRecorder.");
            }
        }

        /// <summary>
        /// Helper function to get the library path of pv_recorder.
        /// </summary>
        /// <returns>A string representing the absolute path of the library.</returns>
        private static string GetLibraryPath()
        {
            string scriptPath;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                scriptPath = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "scripts/platform.bat");
            }
            else
            {
                scriptPath = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "scripts/platform.sh");
            }

            var process = new Process();
            var processStartInfo = new ProcessStartInfo()
            {
                FileName = scriptPath,
                UseShellExecute = false,
                RedirectStandardOutput = true
            };

            process.StartInfo = processStartInfo;
            process.Start();
            process.WaitForExit();

            if (process.ExitCode != 0)
            {
                throw new SystemException("System is not supported.");
            }

            string[] output = process.StandardOutput.ReadToEnd().Split(' ');
            string osName = output[0];
            string cpu = output[1];

            string libName;

            if (osName == "windows")
            {
                libName = $"{LIBRARY}.dll";
            }
            else if (osName == "mac")
            {
                libName = $"{LIBRARY}.dylib";
            }
            else
            {
                libName = $"{LIBRARY}.so";
            }

            return Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), $"lib/{osName}/{cpu}/{libName}");
        }

        /// <summary>
        /// Frees memory used by PvRecorder instance.
        /// </summary>
        public void Dispose()
        {
            if (_libraryPointer != IntPtr.Zero)
            {
                pv_recorder_delete(_libraryPointer);
                _libraryPointer = IntPtr.Zero;

                GC.SuppressFinalize(this);
            }
        }

        ~PvRecorder()
        {
            Dispose();
        }
    }
}