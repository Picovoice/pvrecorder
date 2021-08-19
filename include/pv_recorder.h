/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#ifndef PV_RECORDER_H
#define PV_RECORDER_H

#include <stdbool.h>
#include <stdint.h>

#define PV_API __attribute__((visibility ("default")))

#define PV_RECORDER_DEFAULT_DEVICE_INDEX (-1)

/**
 * Forward declaration of PV_Recorder object. It contains everything related to recording
 * audio, and audio frame information.
 */
typedef struct pv_recorder pv_recorder_t;

/**
 * Status codes.
 */
typedef enum {
    PV_RECORDER_STATUS_SUCCESS = 0,
    PV_RECORDER_STATUS_OUT_OF_MEMORY,
    PV_RECORDER_STATUS_INVALID_ARGUMENT,
    PV_RECORDER_STATUS_INVALID_STATE,
    PV_RECORDER_STATUS_BACKEND_ERROR,
    PV_RECORDER_STATUS_DEVICE_ALREADY_INITIALIZED,
    PV_RECORDER_STATUS_DEVICE_NOT_INITIALIZED,
    PV_RECORDER_STATUS_RUNTIME_ERROR
} pv_recorder_status_t;

/**
 * Constructor for Picovoice Audio Recorder.
 *
 * @param device_index The index of the audio device to use. A value of (-1) will resort to default device.
 * @param frame_length The length of the audio frame buffer to fill.
 * @param callback Callback to run after audio frame is filled with frame_length.
 * @param[out] object Audio Recorder object to initialize.
 * @return Status Code. PV_STATUS_INVALID_ARGUMENT, PV_RECORDER_STATUS_BACKEND_ERROR,
 * PV_RECORDER_STATUS_DEVICE_INITIALIZED or PV_STATUS_OUT_OF_MEMORY on failure.
 */
PV_API pv_recorder_status_t pv_recorder_init(
        int32_t device_index,
        int32_t frame_length,
        void (*callback)(const int16_t *),
        pv_recorder_t **object);

/**
 * Destructor.
 *
 * @param object PV_Recorder object.
 */
PV_API void pv_recorder_delete(pv_recorder_t *object);

/**
 * Starts recording audio. Processes the callback given in the constructor.
 *
 * @param object PV_Recorder object.
 * @returnStatus Status Code. Returns PV_STATUS_INVALID_ARGUMENT, PV_RECORDER_STATUS_DEVICE_NOT_INITIALIZED or PV_STATUS_INVALID_STATE on failure.
 */
PV_API pv_recorder_status_t pv_recorder_start(pv_recorder_t *object);

/**
 * Stops recording audio.
 *
 * @param object PV_Recorder object.
 * @return Status Code. Returns PV_STATUS_INVALID_ARGUMENT, PV_RECORDER_STATUS_DEVICE_NOT_INITIALIZED or PV_STATUS_INVALID_STATE on failure.
 */
PV_API pv_recorder_status_t pv_recorder_stop(pv_recorder_t *object);

/**
 * Gets the input audio devices currently available. Each device name has a separate pointer, so the
 * caller must free each item in the output array individually and free the output array itself.
 * The utility function pv_recorder_free_device_list is provided to free the device list.
 *
 * @param[out] count The number of audio devices.
 * @param[out] devices The output array containing the list of audio devices.
 * @return Status Code. Returns PV_STATUS_OUT_OF_MEMORY, PV_RECORDER_STATUS_BACKEND_ERROR or PV_STATUS_INVALID_ARGUMENT on failure.
 */
PV_API pv_recorder_status_t pv_recorder_get_audio_devices(int32_t *count, char ***devices);

/**
 * Frees the device list initialized by pv_recorder_get_audio_devices. The function does not do
 * any checks, providing correct count and pointer is up to the caller.
 *
 * @param count The number of audio devices.
 * @param devices The array containing the list of audio devices.
 */
PV_API void pv_recorder_free_device_list(int32_t count, char **devices);

/**
 * Provides string representations of status codes.
 *
 * @param status Status code.
 * @return String representation.
 */
PV_API const char *pv_recorder_status_to_string(pv_recorder_status_t status);

#endif //PV_RECORDER_H
