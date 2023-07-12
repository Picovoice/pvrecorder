/*
    Copyright 2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

using System;
using System.IO;
using System.Text;
using System.Threading;

using Pv;

namespace PvRecorderDemo
{
    class Demo
    {
        private static readonly int FRAME_LENGTH = 512;

        /// <summary>
        /// Writes the RIFF header for a file in WAV format
        /// </summary>
        /// <param name="writer">Output stream to WAV file</param>
        /// <param name="channelCount">Number of channels</param>
        /// <param name="bitDepth">Number of bits per sample</param>
        /// <param name="sampleRate">Sampling rate in Hz</param>
        /// <param name="totalSampleCount">Total number of samples written to the file</param>
        private static void WriteWavHeader(BinaryWriter writer, ushort channelCount, ushort bitDepth, int sampleRate, int totalSampleCount)
        {
            if (writer == null)
                return;

            writer.Seek(0, SeekOrigin.Begin);
            writer.Write(Encoding.ASCII.GetBytes("RIFF"));
            writer.Write((bitDepth / 8 * totalSampleCount) + 36);
            writer.Write(Encoding.ASCII.GetBytes("WAVE"));
            writer.Write(Encoding.ASCII.GetBytes("fmt "));
            writer.Write(16);
            writer.Write((ushort)1);
            writer.Write(channelCount);
            writer.Write(sampleRate);
            writer.Write(sampleRate * channelCount * bitDepth / 8);
            writer.Write((ushort)(channelCount * bitDepth / 8));
            writer.Write(bitDepth);
            writer.Write(Encoding.ASCII.GetBytes("data"));
            writer.Write(bitDepth / 8 * totalSampleCount);
        }

        /// <summary>
        /// Lists available audio input devices.
        /// </summary>
        public static void ShowAudioDevices()
        {
            string[] devices = PvRecorder.GetAvailableDevices();
            for (int i = 0; i < devices.Length; i++)
            {
                Console.WriteLine($"index: {i}, device name: {devices[i]}");
            }
        }

        private static readonly string HELP_STR = "Available options: \n" +
            " --output_wav_path: Path to write recorded audio to in WAV format.\n" +
            " --audio_device_index: Index of audio device to use for recording.\n" +
            " --show_audio_devices: Print available recording devices.\n";

        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine(HELP_STR);
                Console.Read();
                return;
            }

            bool showAudioDevices = false;
            int audioDeviceIndex = -1;
            string outputWavPath = null;

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
                else if (args[argIndex] == "--output_wav_path")
                {
                    if (++argIndex < args.Length)
                    {
                        outputWavPath = args[argIndex++];

                    }
                }
                else
                {
                    argIndex++;
                }
            }

            if (showAudioDevices)
            {
                ShowAudioDevices();
                return;
            }

            try
            {
                using (PvRecorder recorder = PvRecorder.Create(FRAME_LENGTH))
                {
                    int totalSamplesWritten = 0;
                    BinaryWriter outputFileWriter = null;
                    if (outputWavPath != null)
                    {
                        outputFileWriter = new BinaryWriter(new FileStream(outputWavPath, FileMode.OpenOrCreate, FileAccess.Write));
                        WriteWavHeader(outputFileWriter, 1, 16, recorder.SampleRate, totalSamplesWritten);
                    }

                    Console.CancelKeyPress += delegate (object sender, ConsoleCancelEventArgs e)
                    {
                        e.Cancel = true;
                        recorder.Stop();
                        Console.WriteLine("Stopping...");
                    };

                    recorder.Start();
                    Console.WriteLine($"Using PvRecorder version: {recorder.Version}");
                    Console.WriteLine($"Using device: {recorder.SelectedDevice}");

                    while (recorder.IsRecording)
                    {
                        short[] frame = recorder.Read();
                        if (outputFileWriter != null)
                        {
                            foreach (short sample in frame)
                            {
                                outputFileWriter.Write(sample);
                            }
                            totalSamplesWritten += frame.Length;
                        }

                        Thread.Yield();
                    }

                    if (outputFileWriter != null)
                    {
                        WriteWavHeader(outputFileWriter, 1, 16, recorder.SampleRate, totalSamplesWritten);
                        outputFileWriter.Flush();
                        outputFileWriter.Dispose();
                        Console.WriteLine($"Output file written to '{outputWavPath}'");
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"Exception: {e.Message}");
            }
        }
    }
}