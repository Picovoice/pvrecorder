/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#ifndef PV_RECORDER_RECORDER_H
#define PV_RECORDER_RECORDER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Forward declaration of PV_Recorder object. It contains everything related to recording
 * audio, and audio frame information.
 */
typedef struct pv_recorder pv_recorder_t;

/**
 * Forward declaration of PV_Recorder callback. This runs everytime the frame length supplied by
 * start function is filled up.
 */
typedef void (*pv_recorder_callback_t)(const int16_t *);

/**
 * Status of PV_Recorder.
 */
typedef enum {
    PV_RECORDER_SUCCESS = 0,
    PV_RECORDER_MA_ERROR,
    PV_RECORDER_OUT_OF_MEMORY,
    PV_RECORDER_INVALID_ARGS
} pv_recorder_status_t;

/**
 * Constructor for PV_Recorder.
 *
 * @param frame_length The length of the audio frame buffer to fill.
 * @param callback Callback to run after audio frame is filled with frame_length.
 * @param[out] object Audio Recorder object to initialize.
 * @return Status Code. Returns PV_RECORDER_MA_ERROR, PV_RECORDER_INIT_FAILURE, or PV_RECORDER_OUT_OF_MEMORY on failure.
 */
pv_recorder_status_t pv_recorder_init(int32_t frame_length, pv_recorder_callback_t callback, pv_recorder_t **object);

/**
 * Destructor.
 *
 * @param object PV_Recorder object.
 */
void pv_recorder_delete(pv_recorder_t *object);

/**
 * Gets the input audio devices currently available.
 *
 * @param object PV_Recorder object.
 * @param[out] device_count The number of audio devices.
 * @param[out] devices An array containing names of audio devices.
 * @return Status Code. Returns PV_RECORDER_MA_ERROR on failure or PV_RECORDER_INVALID_ARGS on failure.
 */
pv_recorder_status_t pv_get_audio_devices(pv_recorder_t *object, int32_t *device_count, const char ***devices);

/**
 * Setter to set the index of audio device.
 *
 * @param object PV_Recorder object.
 * @param device_index The index of audio device to set.
 * @return Status Code. Returns PV_RECORDER_INVALID_ARGS on failure.
 */
pv_recorder_status_t pv_set_audio_device(pv_recorder_t *object, int32_t device_index);

/**
 * Starts recording audio.
 *
 * @param object PV_Recorder object.
 * @returnStatus Status Code. Returns PV_RECORDER_INVALID_ARGS or PV_RECORDER_OUT_OF_MEMORY on failure.
 */
pv_recorder_status_t pv_recorder_start(pv_recorder_t *object);

/**
 * Stops recording audio.
 *
 * @param object PV_Recorder object.
 * @return Status Code. Returns PV_RECORDER_MA_ERROR on failure.
 */
pv_recorder_status_t pv_recorder_stop(pv_recorder_t *object);

/**
 * Getter to get status string.
 *
 * @param status Status code.
 * @return Audio status string.
 */
const char *pv_recorder_status_string(pv_recorder_status_t status);

#endif //PV_RECORDER_RECORDER_H
