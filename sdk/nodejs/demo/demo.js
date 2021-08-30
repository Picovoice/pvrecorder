//
// Copyright 2021 Picovoice Inc.
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

const PvRecorder = require("../index");

let isInterrupted = false;

const inputArguments = {
    showAudioDevices: false,
    audioDeviceIndex: -1,
    rawOutputPath: null,
};

function parseArguments() {
    const argv = process.argv.slice(2);

    for (let i = 0; i < argv.length; i++) {
        if ((argv[i] === "-h") || (argv[i] === "--help")) {
            console.log("Usage: node demo/demo.js [-h | --help]");
            console.log("\t--show_audio_devices Prints the list of available input audio devices.");
            console.log("\t--audio_device_index {AUDIO_DEVICE_INDEX} The audio device to capture audio. If empty, default audio device will be used.");
            console.log("\t--raw_output_path {RAW_OUTPUT_PATH} The path to save save raw audio frames. If empty, saving will be omitted.");
            process.exit();
        } else if (argv[i] === "--show_audio_devices") {
            inputArguments.showAudioDevices = true;
        } else if (argv[i] === "--audio_device_index") {
            if (++i < argv.length) {
                inputArguments.audioDeviceIndex = Number(argv[i]);
                if (isNaN(inputArguments.audioDeviceIndex)) {
                    inputArguments.audioDeviceIndex = -1;
                }
            }
        } else if (argv[i] === "--raw_output_path") {
            if (++i < argv.length) {
                inputArguments.rawOutputPath = argv[i];
            }
        }
    }
}

async function runDemo() {
    parseArguments();

    if (inputArguments.showAudioDevices) {
        const devices = PvRecorder.getAudioDevices();
        for (let i = 0; i < devices.length; i++) {
            console.log(`index: ${i}, device name: ${devices[i]}`)
        }
    } else {
        const frameLength = 512;
        const recorder = new PvRecorder(inputArguments.audioDeviceIndex, frameLength);
        console.log(`Using PvRecorder version: ${recorder.version}`);

        recorder.start();
        console.log(`Using device: ${recorder.getSelectedDevice()}`);

        let stream;
        if (inputArguments.rawOutputPath !== null) {
            stream = fs.createWriteStream(inputArguments.rawOutputPath, {flags: 'w'});
        }

        while (!isInterrupted) {
            const pcm = await recorder.read();
            if (inputArguments.rawOutputPath !== null) {
                stream.write(Buffer.from(pcm.buffer));
            }
        }

        if (inputArguments.rawOutputPath !== null) {
            stream.close();
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
