using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Pv
{
    public enum PVRecorderStatus
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
    public class PVRecorder : IDisposable
    {
        private const string LIBRARY = "libpv_recorder";
        private IntPtr _libraryPointer = IntPtr.Zero;

        private int frameLength;

        static PVRecorder()
        {
#if NETCOREAPP3_1
            NativeLibrary.SetDllImportResolver(typeof(PVRecorder).Assembly, ImportResolver);
#endif
        }

#if NETCOREAPP3_1
        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            IntPtr libHandle = IntPtr.Zero;
            NativeLibrary.TryLoad(GetLibraryPath(), out libHandle);
            return libHandle;
        }
#endif
        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PVRecorderStatus pv_recorder_init(int deviceIndex, int frameLength, int bufferSizeMSec, bool logOverflow, out IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void pv_recorder_delete(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PVRecorderStatus pv_recorder_start(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PVRecorderStatus pv_recorder_stop(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PVRecorderStatus pv_recorder_read(IntPtr handle, out short[] pcm);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr pv_recorder_get_selected_device(IntPtr handle);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern PVRecorderStatus pv_recorder_get_audio_devices(out int count, out IntPtr[] devices);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void pv_recorder_free_device_list(int count, IntPtr[] devices);

        [DllImport(LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr pv_recorder_version();

        public static PVRecorder Create(int deviceIndex, int frameLength, int bufferSizeMSec = 1000, bool logOverflow = true)
        {
            return new PVRecorder(deviceIndex, frameLength, bufferSizeMSec, logOverflow);
        }

        private PVRecorder(int deviceIndex, int frameLength, int bufferSizeMSec, bool logOverflow)
        {
            PVRecorderStatus status = pv_recorder_init(deviceIndex, frameLength, bufferSizeMSec, logOverflow, out _libraryPointer);
            if (status != PVRecorderStatus.SUCCESS)
            {
                throw PVRecorderStatusToException(status);
            }

            this.frameLength = frameLength;
            SelectedDevice = Marshal.PtrToStringAnsi(pv_recorder_get_selected_device(_libraryPointer));
            Version = Marshal.PtrToStringAnsi(pv_recorder_version());
        }

        public void Start()
        {
            PVRecorderStatus status = pv_recorder_start(_libraryPointer);
            if (status != PVRecorderStatus.SUCCESS)
            {
                throw PVRecorderStatusToException(status);
            }
        }

        public void Stop()
        {
            PVRecorderStatus status = pv_recorder_stop(_libraryPointer);
            if (status != PVRecorderStatus.SUCCESS)
            {
                throw PVRecorderStatusToException(status);
            }
        }

        public short[] Read()
        {
            short[] pcm = new short[frameLength];
            PVRecorderStatus status = pv_recorder_read(_libraryPointer, out pcm);
            if (status != PVRecorderStatus.SUCCESS)
            {
                throw PVRecorderStatusToException(status);
            }
            return pcm;
        }

        public string SelectedDevice
        {
            get; private set;
        }

        public string Version
        {
            get; private set;
        }

        public static string[] GetAudioDevices()
        {
            int count;
            IntPtr[] devices;

            PVRecorderStatus status = pv_recorder_get_audio_devices(out count, out devices);
            if (status != PVRecorderStatus.SUCCESS)
            {
                throw PVRecorderStatusToException(status);
            }

            string[] deviceNames = new string[count];
            for (int i = 0; i < count; i++)
            {
                deviceNames[i] = Marshal.PtrToStringAnsi(devices[i]);
            }

            pv_recorder_free_device_list(count, devices);

            return deviceNames;
        }

        private static Exception PVRecorderStatusToException(PVRecorderStatus status, string message=null)
        {
            switch (status)
            {
                default:
                    return new Exception("Unknown status returned from PVRecorder.");
            }
        }

        private static string GetLibraryPath()
        {
            string scriptPath = "scripts/";
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                scriptPath += "platform.bat";
            }
            else
            {
                scriptPath += "platform.sh";
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
                libName = string.Format("{0}.dll", LIBRARY);
            } 
            else if (osName == "mac")
            {
                libName = string.Format("{0}.dll", LIBRARY);
            }
            else
            {
                libName = string.Format("{0}.dll", LIBRARY);
            }

            return string.Format("lib/{0}/{1}/{2}", osName, cpu, libName);
        }

        public void Dispose()
        {
            if (_libraryPointer != IntPtr.Zero)
            {
                pv_recorder_delete(_libraryPointer);
                _libraryPointer = IntPtr.Zero;

                GC.SuppressFinalize(this);
            }
        }

        ~PVRecorder()
        {
            Dispose();
        }
    }
}
