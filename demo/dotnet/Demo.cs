using System;
using System.IO;
using Pv;

namespace demo
{
    class Demo
    {
        private static bool isInterrupted = false;

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
                string[] devices = PvRecorder.GetAudioDevices();
                for (int i = 0; i < devices.Length; i++)
                {
                    Console.WriteLine($"index: {i}, name: {devices[i]}");
                }
                return;
            }
            PvRecorder recorder = null;
            FileStream fileStream = null;
            BinaryWriter binaryWriter = null;

            try
            {
                Console.CancelKeyPress += delegate (object sender, ConsoleCancelEventArgs e)
                {
                    e.Cancel = true;
                    Demo.isInterrupted = true;
                };

                int frameLength = 512;
                recorder = PvRecorder.Create(audioDeviceIndex, frameLength);
                Console.WriteLine($"Using PvRecorder version: {recorder.Version}");

                recorder.Start();
                Console.WriteLine($"Using device: {recorder.SelectedDevice}");

                if (rawOutputPath != null)
                {
                    fileStream = new FileStream(rawOutputPath, FileMode.Create);
                    binaryWriter = new BinaryWriter(fileStream);
                }

                while (!isInterrupted)
                {
                    short[] pcm = recorder.Read();
                    if (binaryWriter != null)
                    {
                        foreach (short frame in pcm)
                        {
                            binaryWriter.Write(frame);
                        }
                    }
                }
            }
            catch (Exception e) {
                Console.WriteLine($"Exception: {e.Message}");
            }
            finally
            {

                Console.WriteLine("Stopping...");
                if (binaryWriter != null)
                {
                    binaryWriter.Close();
                }

                if (fileStream != null)
                {
                    fileStream.Close();
                }
                recorder?.Dispose();
            }
    
        }
    }
}
