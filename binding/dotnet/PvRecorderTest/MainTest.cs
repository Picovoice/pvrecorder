/*
    Copyright 2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

using System;

using Microsoft.VisualStudio.TestTools.UnitTesting;

using Pv;

namespace PvRecorderTest
{
    [TestClass]
    public class MainTest
    {
        private static readonly int FRAME_LENGTH = 512;

        [TestMethod]
        public void TestInit()
        {
            PvRecorder recorder = PvRecorder.Create(FRAME_LENGTH, deviceIndex: 0, bufferedFramesCount: 60);
            Assert.IsNotNull(recorder);
            Assert.IsTrue(recorder.SampleRate > 0);
            Assert.IsFalse(string.IsNullOrEmpty(recorder.SelectedDevice));
            Assert.IsFalse(string.IsNullOrEmpty(recorder.Version));
            recorder.Dispose();
        }

        [TestMethod]
        public void TestStartStop()
        {
            using (PvRecorder recorder = PvRecorder.Create(FRAME_LENGTH, deviceIndex: 0))
            {
                recorder.SetDebugLogging(true);

                Assert.IsFalse(recorder.IsRecording);
                recorder.Start();
                Assert.IsTrue(recorder.IsRecording);

                short[] frame = recorder.Read();
                Assert.IsNotNull(frame);
                Assert.AreEqual(FRAME_LENGTH, frame.Length);

                recorder.Stop();
                Assert.IsFalse(recorder.IsRecording);
            }
        }

        [TestMethod]
        public void TestGetAudioDevices()
        {
            string[] devices = PvRecorder.GetAvailableDevices();

            Assert.IsNotNull(devices);
            Assert.IsTrue(devices.Length >= 0);
            if (devices.Length > 0)
            {
                for (int i = 0; i < devices.Length; i++)
                {
                    Assert.IsNotNull(devices[i]);
                }
            }
        }
    }
}