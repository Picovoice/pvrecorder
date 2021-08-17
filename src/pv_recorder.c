/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <string.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wunused-result"

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio/miniaudio.h"

#pragma GCC diagnostic pop

#include "../include/pv_recorder.h"

#define PV_RECORDER_ASSERT(object) if (!object) return PV_RECORDER_INVALID_ARGS

struct pv_recorder {
    ma_context context;
    ma_device device;
    const char **device_refs;
    int32_t device_count;
    int32_t frame_length;
    int32_t buffer_filled;
    int16_t *frame_buffer;
    pv_recorder_callback_t pcm_callback;
};

const char pv_recorder_success_message[] = "pv_recorder success.";
const char pv_recorder_ma_error_message[] = "pv_recorder miniaudio error occurred.";
const char pv_recorder_out_of_memory_message[] = "pv_recorder failed to allocate memory.";
const char pv_recorder_invalid_args_message[] = "pv_recorder received incorrect params.";

static void pv_recorder_ma_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
    (void) output;

    pv_recorder_t *object = (pv_recorder_t *) device->pUserData;

    int16_t *buffer_ptr = &(object->frame_buffer[object->buffer_filled]);
    int16_t *input_ptr = (int16_t *) input;

    int32_t processed_frames = 0;

    while (processed_frames < frame_count) {
        const int32_t remaining_frames = (int32_t) frame_count - processed_frames;
        const int32_t available_frames = object->frame_length - object->buffer_filled;

        if (available_frames > 0) {
            const int32_t frames_to_read = (remaining_frames < available_frames) ? remaining_frames : available_frames;

            memcpy(buffer_ptr, input_ptr, frames_to_read * sizeof(int16_t));
            buffer_ptr += frames_to_read;
            input_ptr += frames_to_read;

            processed_frames += frames_to_read;
            object->buffer_filled += frames_to_read;
        } else {
            object->pcm_callback(object->frame_buffer);

            buffer_ptr = object->frame_buffer;
            object->buffer_filled = 0;
        }
    }
}

pv_recorder_status_t pv_recorder_init(int32_t frame_length, pv_recorder_callback_t callback, pv_recorder_t **object) {
    if (frame_length <= 0) {
        return PV_RECORDER_INVALID_ARGS;
    }
    PV_RECORDER_ASSERT(callback);
    PV_RECORDER_ASSERT(object);
    *object = NULL;

    pv_recorder_t *recorder = calloc(1, sizeof(pv_recorder_t));
    if (recorder == NULL) {
        return PV_RECORDER_OUT_OF_MEMORY;
    }

    recorder->frame_buffer = malloc(frame_length * sizeof (int16_t));
    if (recorder->frame_buffer == NULL) {
        return PV_RECORDER_OUT_OF_MEMORY;
    }
    recorder->frame_length = frame_length;
    recorder->pcm_callback = callback;

    ma_result result;
    result = ma_context_init(NULL, 0, NULL, &recorder->context);
    if (result != MA_SUCCESS) {
        pv_recorder_delete(recorder);
        return PV_RECORDER_MA_ERROR;
    }

    ma_device_config device_config;
    device_config = ma_device_config_init(ma_device_type_capture);
    device_config.capture.format = ma_format_s16;
    device_config.capture.channels = 1;
    device_config.sampleRate = ma_standard_sample_rate_16000;
    device_config.dataCallback = pv_recorder_ma_callback;
    device_config.pUserData = recorder;

    result = ma_device_init(&recorder->context, &device_config, &recorder->device);
    if (result != MA_SUCCESS) {
        pv_recorder_delete(recorder);
        return PV_RECORDER_MA_ERROR;
    }

    *object = recorder;

    return PV_RECORDER_SUCCESS;
}

void pv_recorder_delete(pv_recorder_t *object) {
    if (object) {
        ma_device_uninit(&object->device);
        ma_context_uninit(&object->context);

        for (int32_t i = 0; i < object->device_count; ++i) {
            free((void *) object->device_refs[i]);
        }
        free(object->device_refs);
        free(object->frame_buffer);
        free(object);
    }
}

pv_recorder_status_t pv_get_audio_devices(pv_recorder_t *object, int32_t *device_count, const char ***devices) {
    PV_RECORDER_ASSERT(object);
    PV_RECORDER_ASSERT(device_count);
    PV_RECORDER_ASSERT(devices);

    if (object->device_refs != NULL) {
        for (int32_t i = 0; i < object->device_count; ++i) {
            free((void *) object->device_refs[i]);
        }
        free(object->device_refs);
        object->device_count = 0;
    }

    ma_device_info *capture_info;
    ma_uint32 count;
    ma_result result = ma_context_get_devices(&object->context, NULL, NULL, &capture_info, &count);
    if (result != MA_SUCCESS) {
        return PV_RECORDER_MA_ERROR;
    }

    const char **device_refs = malloc(count * sizeof(char *));
    if (device_refs == NULL) {
        return PV_RECORDER_OUT_OF_MEMORY;
    }
    for (int32_t i = 0; i < count; ++i) {
        device_refs[i] = strdup(capture_info[i].name);
        if (device_refs[i] == NULL) {
            for (int32_t j = i - 1; j >= 0; --j) {
                free((void *) device_refs[j]);
            }
            return PV_RECORDER_OUT_OF_MEMORY;
        }
    }

    *devices = device_refs;
    object->device_refs = device_refs;

    *device_count = (int32_t) count;
    object->device_count = *device_count;

    return PV_RECORDER_SUCCESS;
}

pv_recorder_status_t pv_set_audio_device(pv_recorder_t *object, int32_t device_index) {
    PV_RECORDER_ASSERT(object);

    ma_device_info *capture_info;
    ma_uint32 count;
    ma_result result = ma_context_get_devices(&object->context, NULL, NULL, &capture_info, &count);
    if (result != MA_SUCCESS) {
        return PV_RECORDER_MA_ERROR;
    }
    if (device_index < 0 || device_index >= count) {
        return PV_RECORDER_INVALID_ARGS;
    }

    object->device.capture.id = capture_info[device_index].id;
    return PV_RECORDER_SUCCESS;
}

pv_recorder_status_t pv_recorder_start(pv_recorder_t *object) {
    PV_RECORDER_ASSERT(object);

    ma_result result;
    result = ma_device_start(&object->device);
    if (result != MA_SUCCESS) {
        return PV_RECORDER_MA_ERROR;
    }

    return PV_RECORDER_SUCCESS;
}

pv_recorder_status_t pv_recorder_stop(pv_recorder_t *object) {
    PV_RECORDER_ASSERT(object);

    ma_result result;
    result = ma_device_stop(&object->device);
    if (result != MA_SUCCESS) {
        return PV_RECORDER_MA_ERROR;
    }

    return PV_RECORDER_SUCCESS;
}

const char *pv_recorder_status_string(pv_recorder_status_t status) {
    if (status == PV_RECORDER_SUCCESS) {
        return pv_recorder_success_message;
    }
    if (status == PV_RECORDER_MA_ERROR) {
        return pv_recorder_ma_error_message;
    }
    if (status == PV_RECORDER_OUT_OF_MEMORY) {
        return pv_recorder_out_of_memory_message;
    }
    if (status == PV_RECORDER_INVALID_ARGS) {
        return pv_recorder_invalid_args_message;
    }
}
