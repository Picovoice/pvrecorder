//
// Copyright 2022 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
// file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
"use strict";

import { execSync } from "child_process";
import * as os from "os";
import * as path from "path";

import PvRecorderStatus from "./pv_recorder_status_t";
import PvRecorderStatusToException from "./errors";

/**
 * PvRecorder class to record audio.
 */
class PvRecorder {
  private static _pvRecorder = require(PvRecorder._getLibraryPath());

  private readonly _handle: number;
  private readonly _frameLength: number;
  private readonly _version: string;

  /**
   * PvRecorder constructor.
   *
   * @param deviceIndex The audio device index to use to record audio. A value of (-1) will use machine's default audio device.
   * @param frameLength Length of the audio frames to receive per read call.
   * @param bufferSizeMSec Time in milliseconds to store the audio frames received.
   * @param logOverflow Boolean indicator to log warnings if the pcm frames buffer received an overflow.
   * @param logSilence Boolean variable to enable silence logs. This will log when continuous audio buffers are detected as silent.
   */
  constructor(
    deviceIndex: number,
    frameLength: number,
    bufferSizeMSec = 1000,
    logOverflow = true,
    logSilence = true
  ) {
    let porcupineHandleAndStatus;
    try {
      porcupineHandleAndStatus = PvRecorder._pvRecorder.init(deviceIndex, frameLength, bufferSizeMSec, logOverflow, logSilence);
    } catch (err: any) {
      PvRecorderStatusToException(err.code, err);
    }
    const status = porcupineHandleAndStatus.status;
    if (status !== PvRecorderStatus.SUCCESS) {
      throw PvRecorderStatusToException(status, "PvRecorder failed to initialize.");
    }
    this._handle = porcupineHandleAndStatus.handle;
    this._frameLength = frameLength;
    this._version = PvRecorder._pvRecorder.version();
  }

  /**
   * @returns Length of the audio frames to receive per read call.
   */
  get frameLength(): number {
    return this._frameLength;
  }

  /**
   * @returns the version of the PvRecorder
   */
  get version(): string {
    return this._version;
  }

  /**
   * Starts recording audio.
   */
  public start(): void {
    const status = PvRecorder._pvRecorder.start(this._handle);
    if (status !== PvRecorderStatus.SUCCESS) {
      throw PvRecorderStatusToException(status, "PvRecorder failed to start.");
    }
  }

  /**
   * Stops recording audio.
   */
  public stop(): void {
    const status = PvRecorder._pvRecorder.stop(this._handle);
    if (status !== PvRecorderStatus.SUCCESS) {
      throw PvRecorderStatusToException(status, "PvRecorder failed to stop.");
    }
  }

  /**
   * Asynchronous call to read pcm frames.
   *
   * @returns {Promise<Int16Array>} Pcm frames.
   */
  public async read(): Promise<Int16Array> {
    return new Promise<Int16Array>((resolve, reject) => {
      setTimeout(() => {
        let pcm = new Int16Array(this._frameLength);
        const status = PvRecorder._pvRecorder.read(this._handle, pcm);
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
  public readSync(): Int16Array {
    let pcm = new Int16Array(this._frameLength);
    const status = PvRecorder._pvRecorder.read(this._handle, pcm);
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
  public getSelectedDevice(): string {
    const device = PvRecorder._pvRecorder.get_selected_device(this._handle);
    if ((device === undefined) || (device === null)) {
      throw new Error("Failed to get selected device.");
    }
    return device;
  }

  /**
   * Destructor. Releases any resources used by PvRecorder.
   */
  public release(): void {
    PvRecorder._pvRecorder.delete(this._handle);
  }

  /**
   * Helper function to get the available devices.
   *
   * @returns {Array<string>} An array of the device names.
   */
  public static getAudioDevices() {
    const devices = PvRecorder._pvRecorder.get_audio_devices();
    if ((devices === undefined) || (devices === null)) {
      throw new Error("Failed to get audio devices.");
    }
    return devices;
  }

  private static _getLibraryPath() {
    let scriptPath;
    if (os.platform() === "win32") {
      scriptPath = path.resolve(__dirname, "..", "scripts", "platform.bat")
    } else {
      scriptPath = path.resolve(__dirname, "..", "scripts", "platform.sh")
    }

    let output = execSync(`"${scriptPath}"`).toString();
    let [osName, cpu] = output.split(" ");

    return path.resolve(__dirname, "..", "lib", osName, cpu, "pv_recorder.node");
  }
}

export default PvRecorder;
