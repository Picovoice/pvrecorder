using System;

using Pv;

namespace demo
{
    class Demo
    {
        static void Main(string[] args)
        {
            bool showAudioDevices = false;
            int audioDeviceIndex = -1;
            string rawOutputPath = null;

            int argIndex = 0;
            while (argIndex < args.Length)
            {
                if (args[argIndex] == "--show_audio_devices")
                {
                    showAudioDevices = true;
                    argIndex++;
                }
                else if (args[argIndex] == "--audio_device_index")
                {
                    if (++argIndex < args.Length && int.TryParse(args[argIndex], out int deviceIndex))
                    {
                        audioDeviceIndex = deviceIndex;
                        argIndex++;
                    }
                }
                else if (args[argIndex] == "--raw_output_path")
                {
                    if (++argIndex < args.Length)
                    {
                        rawOutputPath = args[argIndex++];

                    }
                }
                else
                {
                    argIndex++;
                }
            }

            if (showAudioDevices)
            {
                string[] devices = PVRecorder.GetAudioDevices();
                for (int i = 0; i < devices.Length; i++)
                {
                    Console.WriteLine($"index: {i}, name: {devices[i]}");
                }
            }
            else
            {
                int frameLength = 512;
                PVRecorder recorder = PVRecorder.Create(audioDeviceIndex, frameLength);
                Console.WriteLine($"Using PVRecorder version: {recorder.Version}");

                recorder.Start();
                Console.WriteLine($"Using device: {recorder.SelectedDevice}");

                while (true)
                {
                    short[] pcm = recorder.Read();
                }

            }
        }
    }
}
