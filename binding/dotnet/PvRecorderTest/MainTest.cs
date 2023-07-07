using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

using Microsoft.VisualStudio.TestTools.UnitTesting;

using Pv;

namespace PvRecorderTest
{
    [TestClass]
    public class MainTest
    {
        private static bool _hasAudioDevices;

        [ClassInitialize]
        public static void ClassInitialize(TestContext _)
        {
            _hasAudioDevices = PvRecorder.GetAvailableDevices().Length > 0;
        }

        [TestMethod]
        public void TestInit()
        {
            if (!_hasAudioDevices)
            {
                Assert.Inconclusive("No audio devices to test with.");
                return;
            }

            PvRecorder recorder = PvRecorder.Create(512, deviceIndex: 0, bufferedFramesCount: 60);
            Assert.IsNotNull(recorder);
            Assert.IsTrue(recorder.SampleRate > 0);
            Assert.IsFalse(string.IsNullOrEmpty(recorder.SelectedDevice));
            Assert.IsFalse(string.IsNullOrEmpty(recorder.Version));
            recorder.Dispose();
        }

        [TestMethod]
        public void TestStartStop()
        {
            if (!_hasAudioDevices)
            {
                Assert.Inconclusive("No audio devices to test with.");
                return;
            }

            using (PvRecorder recorder = PvRecorder.Create(512, deviceIndex: 0, bufferedFramesCount: 60))
            {
                recorder.Start();
                short[] frame = recorder.Read();
                recorder.Stop();
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
                    Assert.IsFalse(string.IsNullOrEmpty(devices[i]));
                }
            }
        }
    }
}