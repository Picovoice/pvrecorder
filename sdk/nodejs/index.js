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

const execSync = require("child_process").execSync;
const os = require("os");
const path = require("path");

const pvRecorder = require(getLibraryPath());

const PvRecorderStatus = require("./pv_recorder_status_t");
const PvRecorderStatusToException = require("./errors");

/**
 * PvRecorder class to record audio.
 */
class PvRecorder {

    /**
     * PvRecorder constructor.
     *
     * @param deviceIndex The audio device index to use to record audio. A value of (-1) will use machine's default audio device.
     * @param frameLength Length of the audio frames to receive per read call.
     * @param bufferSizeMSec Time in milliseconds to store the audio frames received.
     * @param logOverflow Boolean indicator to log warnings if the pcm frames buffer received an overflow.
     * @param logSilence Boolean variable to enable silence logs. This will log when continuous audio buffers are detected as silent.
     */
    constructor(deviceIndex, frameLength, bufferSizeMSec = 1000, logOverflow = true, logSilence = true) {
        let porcupineHandleAndStatus;
        try {
            porcupineHandleAndStatus = pvRecorder.init(deviceIndex, frameLength, bufferSizeMSec, logOverflow, logSilence);
        } catch (err) {
            PvRecorderStatusToException(err.code, err);
        }
        const status = porcupineHandleAndStatus.status;
        if (status !== PvRecorderStatus.SUCCESS) {
            throw PvRecorderStatusToException(status, "PvRecorder failed to initialize.");
        }
        this.handle = porcupineHandleAndStatus.handle;
        this.frameLength = frameLength;
        this.version = pvRecorder.version();
    }

    /**
     * Starts recording audio.
     */
    start() {
        const status = pvRecorder.start(this.handle);
        if (status !== PvRecorderStatus.SUCCESS) {
            throw PvRecorderStatusToException(status, "PvRecorder failed to start.");
        }
    }

    /**
     * Stops recording audio.
     */
    stop() {
        const status = pvRecorder.stop(this.handle);
        if (status !== PvRecorderStatus.SUCCESS) {
            throw PvRecorderStatusToException(status, "PvRecorder failed to stop.");
        }
    }

    /**
     * Asynchronous call to read pcm frames.
     *
     * @returns {Promise<Int16Array>} Pcm frames.
     */
    async read() {
        const that = this;
        return new Promise(function (resolve, reject) {
            setTimeout(function () {
                let pcm = new Int16Array(that.frameLength);
                const status = pvRecorder.read(that.handle, pcm);
                if (status !== PvRecorderStatus.SUCCESS) {
                    reject(PvRecorderStatusToException(status, "PvRecorder failed to read pcm frames."));
                }
                resolve(pcm);
            })
        });
    }

    /**
     * Synchronous call to read pcm frames.
     *
     * @returns {Int16Array} Pcm frames.
     */
    readSync() {
        let pcm = new Int16Array(this.frameLength);
        const status = pvRecorder.read(this.handle, pcm);
        if (status !== PvRecorderStatus.SUCCESS) {
            throw PvRecorderStatusToException(status, "PvRecorder failed to read pcm frames.");
        }
        return pcm;
    }

    /**
     * Returns the name of the selected device used to capture audio.
     *
     * @returns {string} Name of the selected audio device.
     */
    getSelectedDevice() {
        const device = pvRecorder.get_selected_device(this.handle);
        if ((device === undefined) || (device === null)) {
            throw new Error("Failed to get selected device.");
        }
        return device;
    }

    /**
     * Destructor. Releases any resources used by PvRecorder.
     */
    release() {
        pvRecorder.delete(this.handle);
    }

    /**
     * Helper function to get the available devices.
     *
     * @returns {Array<string>} An array of the device names.
     */
    static getAudioDevices() {
        const devices = pvRecorder.get_audio_devices();
        if ((devices === undefined) || (devices === null)) {
            throw new Error("Failed to get audio devices.");
        }
        return devices;
    }
}

function getLibraryPath() {
    let scriptPath;
    if (os.platform() === "win32") {
        scriptPath = path.resolve(__dirname, "scripts", "platform.bat")
    } else {
        scriptPath = path.resolve(__dirname, "scripts", "platform.sh")
    }

    let output = execSync(scriptPath).toString();
    let [osName, cpu] = output.split(" ");

    return path.resolve(__dirname, "lib", osName, cpu, "pv_recorder.node");
}

module.exports = PvRecorder;
