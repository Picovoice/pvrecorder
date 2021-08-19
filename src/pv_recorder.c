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

#include "miniaudio.h"

#pragma GCC diagnostic pop

#include "pv_recorder.h"

struct pv_recorder {
    ma_context context;
    ma_device device;
    int32_t frame_length;
    int32_t buffer_filled;
    int16_t *frame_buffer;
    void (*pcm_callback)(const int16_t *);
};

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

PV_API pv_recorder_status_t pv_recorder_init(
        int32_t device_index,
        int32_t frame_length,
        void (*callback)(const int16_t *),
        pv_recorder_t **object) {
    if (frame_length <= 0) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!callback) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (device_index < PV_RECORDER_DEFAULT_INDEX) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    *object = NULL;

    pv_recorder_t *o = calloc(1, sizeof(pv_recorder_t));
    if (!o) {
        return PV_RECORDER_STATUS_OUT_OF_MEMORY;
    }

    ma_result result = ma_context_init(NULL, 0, NULL, &(o->context));
    if (result != MA_SUCCESS) {
        pv_recorder_delete(o);
        if ((result == MA_NO_BACKEND) || (result == MA_FAILED_TO_INIT_BACKEND)) {
            return PV_RECORDER_STATUS_BACKEND_ERROR;
        } else if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    ma_device_config device_config;
    device_config = ma_device_config_init(ma_device_type_capture);
    device_config.capture.format = ma_format_s16;
    device_config.capture.channels = 1;
    device_config.sampleRate = ma_standard_sample_rate_16000;
    device_config.dataCallback = pv_recorder_ma_callback;
    device_config.pUserData = o;

    if (device_index != PV_RECORDER_DEFAULT_INDEX) {
        ma_device_info *capture_info = NULL;
        ma_uint32 count = 0;
        result = ma_context_get_devices(&(o->context), NULL, NULL, &capture_info, &count);
        if (result != MA_SUCCESS) {
            pv_recorder_delete(o);
            if (result == MA_OUT_OF_MEMORY) {
                return PV_RECORDER_STATUS_OUT_OF_MEMORY;
            } else {
                return PV_RECORDER_STATUS_INVALID_STATE;
            }
        }
        if (device_index >= count) {
            pv_recorder_delete(o);
            return PV_RECORDER_STATUS_INVALID_ARGUMENT;
        }
        device_config.capture.pDeviceID = &capture_info[device_index].id;
    }

    result = ma_device_init(&(o->context), &device_config, &(o->device));
    if (result != MA_SUCCESS) {
        pv_recorder_delete(o);
        if (result == MA_DEVICE_ALREADY_INITIALIZED) {
            return PV_RECORDER_STATUS_DEVICE_INITIALIZED;
        } else if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    o->frame_buffer = malloc(frame_length * sizeof(int16_t));
    if (!(o->frame_buffer)) {
        pv_recorder_delete(o);
        return PV_RECORDER_STATUS_OUT_OF_MEMORY;
    }
    o->frame_length = frame_length;
    o->pcm_callback = callback;

    *object = o;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API void pv_recorder_delete(pv_recorder_t *object) {
    if (object) {
        ma_device_uninit(&(object->device));
        ma_context_uninit(&(object->context));
        free(object->frame_buffer);
        free(object);
    }
}

PV_API pv_recorder_status_t pv_recorder_start(pv_recorder_t *object) {
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    ma_result result = ma_device_start(&(object->device));
    if (result != MA_SUCCESS) {
        if (result == MA_DEVICE_NOT_INITIALIZED) {
            return PV_RECORDER_STATUS_DEVICE_NOT_INITIALIZED;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API pv_recorder_status_t pv_recorder_stop(pv_recorder_t *object) {
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    ma_result result = ma_device_stop(&(object->device));
    if (result != MA_SUCCESS) {
        if (result == MA_DEVICE_NOT_INITIALIZED) {
            return PV_RECORDER_STATUS_DEVICE_NOT_INITIALIZED;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API pv_recorder_status_t pv_recorder_get_audio_devices(int32_t *count, char ***devices) {
    if (!count) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!devices) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    ma_context context;
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        if ((result == MA_NO_BACKEND) || (result == MA_FAILED_TO_INIT_BACKEND)) {
            return PV_RECORDER_STATUS_BACKEND_ERROR;
        } else if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    ma_device_info *capture_info;
    ma_uint32 capture_count;
    result = ma_context_get_devices(&context, NULL, NULL, &capture_info, &capture_count);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    char **d = calloc(capture_count, sizeof(char *));
    if (!d) {
        ma_context_uninit(&context);
        return PV_RECORDER_STATUS_OUT_OF_MEMORY;
    }

    for (int32_t i = 0; i < (int32_t) capture_count; i++) {
        d[i] = strdup(capture_info[i].name);
        if (!d[i]) {
            for (int32_t j = i - 1; j >= 0; j--) {
                free(d[j]);
            }
            free(d);
            ma_context_uninit(&context);
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        }
    }

    ma_context_uninit(&context);

    *count = (int32_t) capture_count;
    *devices = d;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API void pv_recorder_free_device_list(int32_t count, char **devices) {
    if (devices && (count > 0)) {
        for (int32_t i = 0; i < count; i++) {
            free(devices[i]);
        }
        free(devices);
    }
}

PV_API const char *pv_recorder_status_to_string(pv_recorder_status_t status) {
    static const char *const STRINGS[] = {
            "SUCCESS",
            "OUT_OF_MEMORY",
            "INVALID_ARGUMENT",
            "INVALID_STATE",
            "BACKEND_ERROR",
            "DEVICE_INITIALIZED",
            "DEVICE_NOT_INITIALIZED"};

    int32_t size = sizeof(STRINGS)/sizeof(STRINGS[0]);
    if (status < PV_RECORDER_STATUS_SUCCESS || status >= (PV_RECORDER_STATUS_SUCCESS + size)) {
        return NULL;
    }

    return STRINGS[status - PV_RECORDER_STATUS_SUCCESS];
}
