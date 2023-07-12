#! /usr/bin/env node

// Copyright 2021-2023 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
// file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
"use strict";

const fs = require("fs");
const { program } = require("commander");
const { WaveFile } = require("wavefile");

const { PvRecorder } = require("@picovoice/pvrecorder-node");

let isInterrupted = false;

program
.option(
  "-i, --audio_device_index <number>",
  "index of audio device to use to record audio",
  Number,
  -1
).option(
  "-d, --show_audio_devices",
  "show the list of available devices"
).option(
  "-o, --output_wav_path <string>",
  "output_path to save wav file"
);

if (process.argv.length < 2) {
  program.help();
}
program.parse(process.argv);

async function runDemo() {
  let audioDeviceIndex = program["audio_device_index"];
  let showAudioDevices = program["show_audio_devices"];
  let outputWavPath = program["output_wav_path"];

  if (showAudioDevices) {
    const devices = PvRecorder.getAvailableDevices();
    for (let i = 0; i < devices.length; i++) {
      console.log(`index: ${i}, device name: ${devices[i]}`)
    }
  } else {
    const wav = new WaveFile();
    const frames = [];

    const frameLength = 512;
    const recorder = new PvRecorder(frameLength, audioDeviceIndex);
    console.log(`Using PvRecorder version: ${recorder.version}`);

    recorder.start();
    console.log(`Using device: ${recorder.getSelectedDevice()}`);

    while (!isInterrupted) {
      const frame = await recorder.read();
      if (outputWavPath) {
        frames.push(frame);
      }
    }

    if (outputWavPath) {
      const audioData = new Int16Array(recorder.frameLength * frames.length);
      for (let i = 0; i < frames.length; i++) {
        audioData.set(frames[i], i * recorder.frameLength);
      }

      wav.fromScratch(1, recorder.sampleRate, '16', audioData);
      fs.writeFileSync(outputWavPath, wav.toBuffer());
    }

    console.log("Stopping...");
    recorder.release();
  }
}

// setup interrupt
process.on("SIGINT", function () {
  isInterrupted = true;
});

(async function () {
  try {
    await runDemo();
  } catch (e) {
    console.error(e.toString());
  }
})();
