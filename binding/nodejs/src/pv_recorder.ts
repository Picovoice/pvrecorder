//
// Copyright 2022-2023 Picovoice Inc.
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
import pvRecorderStatusToException from "./errors";
import { getSystemLibraryPath } from './platforms';

/**
 * PvRecorder class for recording audio.
 */
class PvRecorder {
  // eslint-disable-next-line
  private static _pvRecorder = require(getSystemLibraryPath());

  private readonly _handle: number;
  private readonly _frameLength: number;
  private readonly _sampleRate: number;
  private readonly _version: string;

  /**
   * PvRecorder constructor.
   *
   * @param frameLength Length of the audio frames to receive per read call.
   * @param deviceIndex The audio device index to use to record audio. A value of (-1) will use machine's default audio device.
   * @param bufferedFramesCount The number of audio frames buffered internally for reading - i.e. internal circular buffer
   * will be of size `frameLength` * `bufferedFramesCount`. If this value is too low, buffer overflows could occur
   * and audio frames could be dropped. A higher value will increase memory usage.
   */
  constructor(
    frameLength: number,
    deviceIndex: number = -1,
    bufferedFramesCount = 50,
  ) {
    let pvRecorderHandleAndStatus;
    try {
      pvRecorderHandleAndStatus = PvRecorder._pvRecorder.init(frameLength, deviceIndex, bufferedFramesCount);
    } catch (err: any) {
      pvRecorderStatusToException(err.code, err);
    }
    const status = pvRecorderHandleAndStatus.status;
    if (status !== PvRecorderStatus.SUCCESS) {
      throw pvRecorderStatusToException(status, "PvRecorder failed to initialize.");
    }
    this._handle = pvRecorderHandleAndStatus.handle;
    this._frameLength = frameLength;
    this._sampleRate = PvRecorder._pvRecorder.sample_rate();
    this._version = PvRecorder._pvRecorder.version();
  }

  /**
   * @returns Length of the audio frames to receive per read call.
   */
  get frameLength(): number {
    return this._frameLength;
  }

  /**
   * @returns Audio sample rate used by PvRecorder.
   */
  get sampleRate(): number {
    return this._sampleRate;
  }

  /**
   * @returns the version of the PvRecorder
   */
  get version(): string {
    return this._version;
  }

  /**
   * @returns Whether PvRecorder is currently recording audio or not.
   */
  get isRecording(): boolean {
    return PvRecorder._pvRecorder.get_is_recording(this._handle);
  }

  /**
   * Starts recording audio.
   */
  public start(): void {
    const status = PvRecorder._pvRecorder.start(this._handle);
    if (status !== PvRecorderStatus.SUCCESS) {
      throw pvRecorderStatusToException(status, "PvRecorder failed to start.");
    }
  }

  /**
   * Stops recording audio.
   */
  public stop(): void {
    const status = PvRecorder._pvRecorder.stop(this._handle);
    if (status !== PvRecorderStatus.SUCCESS) {
      throw pvRecorderStatusToException(status, "PvRecorder failed to stop.");
    }
  }

  /**
   * Asynchronous call to read a frame of audio data.
   *
   * @returns {Promise<Int16Array>} Audio data frame.
   */
  public async read(): Promise<Int16Array> {
    return new Promise<Int16Array>((resolve, reject) => {
      setTimeout(() => {
        const pcm = new Int16Array(this._frameLength);
        const status = PvRecorder._pvRecorder.read(this._handle, pcm);
        if (status !== PvRecorderStatus.SUCCESS) {
          reject(pvRecorderStatusToException(status, "PvRecorder failed to read audio data frame."));
        }
        resolve(pcm);
      });
    });
  }

  /**
   * Synchronous call to read a frame of audio data.
   *
   * @returns {Int16Array} Audio data frame.
   */
  public readSync(): Int16Array {
    const pcm = new Int16Array(this._frameLength);
    const status = PvRecorder._pvRecorder.read(this._handle, pcm);
    if (status !== PvRecorderStatus.SUCCESS) {
      throw pvRecorderStatusToException(status, "PvRecorder failed to read audio data frame.");
    }
    return pcm;
  }

  /**
   * Enable or disable debug logging for PvRecorder. Debug logs will indicate when there are overflows in the internal
   * frame buffer and when an audio source is generating frames of silence.
   *
   * @param isDebugLoggingEnabled Boolean indicating whether the debug logging is enabled or disabled.
   */
  public setDebugLogging(isDebugLoggingEnabled: boolean): void {
    PvRecorder._pvRecorder.set_debug_logging(this._handle, isDebugLoggingEnabled);
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
   * Destructor. Releases resources acquired by PvRecorder.
   */
  public release(): void {
    PvRecorder._pvRecorder.delete(this._handle);
  }

  /**
   * Helper function to get the list of available audio devices.
   *
   * @returns {Array<string>} An array of the available device names.
   */
  public static getAvailableDevices(): string[] {
    const devices = PvRecorder._pvRecorder.get_available_devices();
    if ((devices === undefined) || (devices === null)) {
      throw new Error("Failed to get audio devices.");
    }
    return devices;
  }
}

export default PvRecorder;
