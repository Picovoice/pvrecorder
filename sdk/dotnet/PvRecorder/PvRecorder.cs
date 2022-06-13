/*
    Copyright 2021-2022 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/
using System;
using System.IO;
using System.Diagnostics;
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
    /// .NET sdk for capturing and reading audio frames.
    /// </summary>
    public class PvRecorder : IDisposable
    {
        private const string LIBRARY = "libpv_recorder";
        private IntPtr _libraryPointer = IntPtr.Zero;

        private int frameLength;

        static PvRecorder()
        {
#if NETCOREAPP3_1_OR_GREATER
            NativeLibrary.SetDllImportResolver(typeof(PvRecorder).Assembly, ImportResolver);
#endif
        }

#if NETCOREAPP3_1_OR_GREATER
        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            IntPtr libHandle = IntPtr.Zero;
            NativeLibrary.TryLoad(GetLibraryPath(), out libHandle);
            return libHandle;
        }
#endif
        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PvRecorderStatus pv_recorder_init(int deviceIndex, int frameLength, int bufferSizeMSec, bool logOverflow, bool logSilence, out IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void pv_recorder_delete(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PvRecorderStatus pv_recorder_start(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PvRecorderStatus pv_recorder_stop(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PvRecorderStatus pv_recorder_read(IntPtr handle, short[] pcm);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr pv_recorder_get_selected_device(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PvRecorderStatus pv_recorder_get_audio_devices(out int count, out IntPtr devices);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void pv_recorder_free_device_list(int count, IntPtr devices);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr pv_recorder_version();

        /// <summary>
        /// Factory method for PvRecorder library.
        /// </summary>
        /// <param name="deviceIndex">
        /// The index of the audio device to capture audio from. A value of (-1) will use the default audio.
        /// </param>
        /// <param name="frameLength">
        /// Length of the audio frames to receive at each read call.
        /// </param>
        /// <param name="bufferSizeMSec">
        /// Time in milliseconds for the buffer size to hold recorded audio.
        /// </param>
        /// <param name="logOverflow">
        /// Boolean value representing if buffer overflow warnings should be logged.
        /// </param>
        /// <param name="logSilence">
        /// Boolean variable to enable silence logs. This will log when continuous audio buffers are detected as silent.
        /// </param>
        /// <returns>An instance of PvRecorder.</returns>
        public static PvRecorder Create(int deviceIndex, int frameLength, int bufferSizeMSec = 1000, bool logOverflow = true, bool logSilence = true)
        {
            return new PvRecorder(deviceIndex, frameLength, bufferSizeMSec, logOverflow, logSilence);
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
        /// <param name="bufferSizeMSec">
        /// Time in milliseconds for the buffer size to hold recorded audio.
        /// </param>
        /// <param name="logOverflow">
        /// Boolean value representing if buffer overflow warnings should be logged.
        /// </param>
        /// <param name="logSilence">
        /// Boolean variable to enable silence logs. This will log when continuous audio buffers are detected as silent.
        /// </param>
        private PvRecorder(int deviceIndex, int frameLength, int bufferSizeMSec, bool logOverflow, bool logSilence)
        {
            PvRecorderStatus status = pv_recorder_init(deviceIndex, frameLength, bufferSizeMSec, logOverflow, logSilence, out _libraryPointer);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }

            this.frameLength = frameLength;
            SelectedDevice = Marshal.PtrToStringAnsi(pv_recorder_get_selected_device(_libraryPointer));
            Version = Marshal.PtrToStringAnsi(pv_recorder_version());
        }

        /// <summary>
        /// Starts recording audio.
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
        /// Stops recording audio.
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
        /// Reads audio frames.
        /// </summary>
        /// <returns>An array of audio frames with length ${frameLength} provided in the factory method,</returns>
        public short[] Read()
        {
            short[] pcm = new short[frameLength];
            PvRecorderStatus status = pv_recorder_read(_libraryPointer, pcm);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }
            return pcm;
        }

        /// <summary>
        /// Gets the current selected device.
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
        /// Gets the available input devices of the current machine.
        /// </summary>
        /// <returns>A list of strings containing the names of the audio devices.</returns>
        public static string[] GetAudioDevices()
        {
            int count;
            IntPtr devices;

            PvRecorderStatus status = pv_recorder_get_audio_devices(out count, out devices);
            if (status != PvRecorderStatus.SUCCESS)
            {
                throw PvRecorderStatusToException(status);
            }

            int elementSize = Marshal.SizeOf(typeof(IntPtr));
            string[] deviceNames = new string[count];
            for (int i = 0; i < count; i++)
            {
                deviceNames[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(devices, i * elementSize));
            }

            pv_recorder_free_device_list(count, devices);

            return deviceNames;
        }

        /// <summary>
        /// Converts status codes to .NET exceptions.
        /// </summary>
        /// <param name="status">Status code.</param>
        /// <returns>.NET exception</returns>
        private static Exception PvRecorderStatusToException(PvRecorderStatus status)
        {
            switch (status)
            {
                case PvRecorderStatus.OUT_OF_MEMORY:
                    return new OutOfMemoryException();
                case PvRecorderStatus.INVALID_ARGUMENT:
                    return new ArgumentException();
                case PvRecorderStatus.INVALID_STATE:
                    return new Exception("PvRecorder failed with invalid state.");
                case PvRecorderStatus.BACKEND_ERROR:
                    return new Exception("PvRecorder backend error.");
                case PvRecorderStatus.DEVICE_ALREADY_INITIALIZED:
                    return new Exception("PvRecorder device already initialized.");
                case PvRecorderStatus.DEVICE_NOT_INITIALIZED:
                    return new Exception("PvRecorder device not initialized.");
                case PvRecorderStatus.IO_ERROR:
                    return new IOException();
                case PvRecorderStatus.RUNTIME_ERROR:
                    return new SystemException("PvRecorder runtime error.");
                default:
                    return new Exception("Unknown status returned from PvRecorder.");
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

            if (process.ExitCode != 0 )
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
