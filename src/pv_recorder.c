/*
    Copyright 2021-2022 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wunused-result"

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"

#pragma GCC diagnostic pop

#include "pv_circular_buffer.h"
#include "pv_recorder.h"

static const int32_t READ_RETRY_COUNT = 500;
static const int32_t READ_SLEEP_MILLI_SECONDS = 2;
static const int32_t MAX_SILENCE_BUFFER_SIZE = 2 * 16000;
static const int32_t ABSOLUTE_SILENCE_THRESHOLD = 1;

struct pv_recorder {
    ma_context context;
    ma_device device;
    pv_circular_buffer_t *buffer;
    int32_t frame_length;
    int32_t current_silent_samples;
    bool is_started;
    bool log_overflow;
    bool log_silence;
    ma_mutex mutex;
};

static void pv_recorder_ma_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
    (void) output;

    pv_recorder_t *object = (pv_recorder_t *) device->pUserData;

    ma_mutex_lock(&object->mutex);
    pv_circular_buffer_status_t status = pv_circular_buffer_write(object->buffer, input, (int32_t) frame_count);
    if ((status == PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW) && (object->log_overflow)) {
        fprintf(stdout, "[WARN] Overflow - reader is not reading fast enough.\n");
    }

    ma_mutex_unlock(&object->mutex);
}

PV_API pv_recorder_status_t pv_recorder_init(
        int32_t device_index,
        int32_t frame_length,
        int32_t buffer_size_msec,
        bool log_overflow,
        bool log_silence,
        pv_recorder_t **object) {
    if (device_index < PV_RECORDER_DEFAULT_DEVICE_INDEX) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (frame_length <= 0) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (buffer_size_msec <= 0) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    // capacity = 16kHz * seconds
    const int32_t capacity = (int32_t) ((16000 * buffer_size_msec) / 1000);
    if (capacity < frame_length) {
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
            return PV_RECORDER_STATUS_RUNTIME_ERROR;
        }
    }

    ma_device_config device_config;
    device_config = ma_device_config_init(ma_device_type_capture);
    device_config.capture.format = ma_format_s16;
    device_config.capture.channels = 1;
    device_config.sampleRate = ma_standard_sample_rate_16000;
    device_config.dataCallback = pv_recorder_ma_callback;
    device_config.pUserData = o;

    if (device_index != PV_RECORDER_DEFAULT_DEVICE_INDEX) {
        ma_device_info *capture_info = NULL;
        ma_uint32 count = 0;
        result = ma_context_get_devices(&(o->context), NULL, NULL, &capture_info, &count);
        if (result != MA_SUCCESS) {
            pv_recorder_delete(o);
            if (result == MA_OUT_OF_MEMORY) {
                return PV_RECORDER_STATUS_OUT_OF_MEMORY;
            } else {
                return PV_RECORDER_STATUS_RUNTIME_ERROR;
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
            return PV_RECORDER_STATUS_DEVICE_ALREADY_INITIALIZED;
        } else if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_RUNTIME_ERROR;
        }
    }

    result = ma_mutex_init(&(o->mutex));
    if (result != MA_SUCCESS) {
        pv_recorder_delete(o);
        if (result == MA_OUT_OF_MEMORY) {
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        } else {
            return PV_RECORDER_STATUS_RUNTIME_ERROR;
        }
    }

    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            capacity,
            sizeof(int16_t),
            &(o->buffer));

    if (status != PV_CIRCULAR_BUFFER_STATUS_SUCCESS) {
        pv_recorder_delete(o);
        return PV_RECORDER_STATUS_OUT_OF_MEMORY;
    }

    o->frame_length = frame_length;
    o->log_overflow = log_overflow;
    o->log_silence = log_silence;

    *object = o;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API void pv_recorder_delete(pv_recorder_t *object) {
    if (object) {
        ma_device_uninit(&(object->device));
        ma_context_uninit(&(object->context));
        ma_mutex_uninit(&(object->mutex));
        pv_circular_buffer_delete(object->buffer);
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
            // device already started
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    object->is_started = true;

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
            // device already stopped
            return PV_RECORDER_STATUS_INVALID_STATE;
        }
    }

    pv_circular_buffer_reset(object->buffer);
    object->is_started = false;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API pv_recorder_status_t pv_recorder_read(pv_recorder_t *object, int16_t *pcm) {
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!pcm) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!(object->is_started)) {
        return PV_RECORDER_STATUS_INVALID_STATE;
    }

    int16_t *read_ptr = pcm;
    int32_t processed = 0;
    int32_t remaining = object->frame_length;

    for (int32_t i = 0; i < READ_RETRY_COUNT; i++) {
        ma_mutex_lock(&object->mutex);

        const int32_t length = pv_circular_buffer_read(object->buffer, read_ptr, remaining);
        processed += length;

        if (processed == object->frame_length) {
            ma_mutex_unlock(&object->mutex);

            if (object->log_silence) {
                for (int32_t j = 0; j < object->frame_length; j++) {
                    if ((pcm[j] > ABSOLUTE_SILENCE_THRESHOLD) || (pcm[j] < -ABSOLUTE_SILENCE_THRESHOLD)) {
                        object->current_silent_samples = 0;
                        return PV_RECORDER_STATUS_SUCCESS;
                    }
                }
                object->current_silent_samples += object->frame_length;

                if (object->current_silent_samples >= MAX_SILENCE_BUFFER_SIZE) {
                    fprintf(stdout, "[WARN] Input device might be muted or volume level is set to 0.\n");
                    object->current_silent_samples = 0;
                }
            }

            return PV_RECORDER_STATUS_SUCCESS;
        }

        ma_mutex_unlock(&object->mutex);
        ma_sleep(READ_SLEEP_MILLI_SECONDS);

        read_ptr += length;
        remaining = object->frame_length - processed;
    }

    return PV_RECORDER_STATUS_IO_ERROR;
}

PV_API const char *pv_recorder_get_selected_device(pv_recorder_t *object) {
    if (!object) {
        return NULL;
    }
    return object->device.capture.name;
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
            "DEVICE_NOT_INITIALIZED",
            "IO_ERROR",
            "RUNTIME_ERROR"};

    int32_t size = sizeof(STRINGS) / sizeof(STRINGS[0]);
    if (status < PV_RECORDER_STATUS_SUCCESS || status >= (PV_RECORDER_STATUS_SUCCESS + size)) {
        return NULL;
    }

    return STRINGS[status - PV_RECORDER_STATUS_SUCCESS];
}

PV_API const char *pv_recorder_version(void) {
    return "v1.1.0";
}
