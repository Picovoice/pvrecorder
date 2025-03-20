/*
    Copyright 2021-2023 Picovoice Inc.

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

#define PV_RECORDER_DEFAULT_DEVICE_INDEX (-1)
#define PV_RECORDER_SAMPLE_RATE (16000)
#define PV_RECORDER_VERSION "1.2.0"

static const int32_t READ_RETRY_COUNT = 500;
static const int32_t READ_SLEEP_MILLI_SECONDS = 2;
static const int32_t MAX_SILENCE_BUFFER_SIZE = 2 * PV_RECORDER_SAMPLE_RATE;
static const int32_t ABSOLUTE_SILENCE_THRESHOLD = 1;

struct pv_recorder {
    ma_context context;
    ma_device_config device_config;
    ma_device device;
    pv_circular_buffer_t *buffer;
    int32_t frame_length;
    int32_t current_silent_samples;
    bool is_started;
    bool is_debug_logging_enabled;
    ma_mutex mutex;
};

static void pv_recorder_ma_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
    (void) output;

    pv_recorder_t *object = (pv_recorder_t *) device->pUserData;

    ma_mutex_lock(&object->mutex);
    pv_circular_buffer_status_t status = pv_circular_buffer_write(object->buffer, input, (int32_t) frame_count);
    if ((status == PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW) && (object->is_debug_logging_enabled)) {
        fprintf(stdout, "[WARN] Overflow - reader is not reading fast enough.\n");
    }

    ma_mutex_unlock(&object->mutex);
}

static pv_recorder_status_t ma_result_to_pv_recorder_status(ma_result result) {
    switch (result) {
        case MA_SUCCESS:
            return PV_RECORDER_STATUS_SUCCESS;
        case MA_NO_BACKEND:
        case MA_FAILED_TO_INIT_BACKEND:
            return PV_RECORDER_STATUS_BACKEND_ERROR;
        case MA_OUT_OF_MEMORY:
            return PV_RECORDER_STATUS_OUT_OF_MEMORY;
        case MA_DEVICE_ALREADY_INITIALIZED:
            return PV_RECORDER_STATUS_DEVICE_ALREADY_INITIALIZED;
        default:
            return PV_RECORDER_STATUS_RUNTIME_ERROR;
    }
}

PV_API pv_recorder_status_t pv_recorder_init(
        int32_t frame_length,
        int32_t device_index,
        int32_t buffered_frames_count,
        pv_recorder_t **object) {
    if (device_index < PV_RECORDER_DEFAULT_DEVICE_INDEX) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (frame_length <= 0) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (buffered_frames_count < 1) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!object) {
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
        return ma_result_to_pv_recorder_status(result);
    }

    o->device_config = ma_device_config_init(ma_device_type_capture);
    o->device_config.capture.format = ma_format_s16;
    o->device_config.capture.channels = 1;
    o->device_config.sampleRate = ma_standard_sample_rate_16000;
    o->device_config.dataCallback = pv_recorder_ma_callback;
    o->device_config.pUserData = o;

    if (device_index != PV_RECORDER_DEFAULT_DEVICE_INDEX) {
        ma_device_info *capture_info = NULL;
        ma_uint32 count = 0;
        result = ma_context_get_devices(
                &(o->context),
                NULL,
                NULL,
                &capture_info,
                &count);
        if (result != MA_SUCCESS) {
            pv_recorder_delete(o);
            return ma_result_to_pv_recorder_status(result);
        }
        if (device_index >= count) {
            pv_recorder_delete(o);
            return PV_RECORDER_STATUS_INVALID_ARGUMENT;
        }
        o->device_config.capture.pDeviceID = &capture_info[device_index].id;
    }

    result = ma_device_init(&(o->context), &(o->device_config), &(o->device));
    if (result != MA_SUCCESS) {
        pv_recorder_delete(o);
        return ma_result_to_pv_recorder_status(result);
    }

    result = ma_mutex_init(&(o->mutex));
    if (result != MA_SUCCESS) {
        pv_recorder_delete(o);
        return ma_result_to_pv_recorder_status(result);
    }

    const int32_t buffer_capacity = frame_length * buffered_frames_count;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            buffer_capacity,
            sizeof(int16_t),
            &(o->buffer));

    if (status != PV_CIRCULAR_BUFFER_STATUS_SUCCESS) {
        pv_recorder_delete(o);
        return PV_RECORDER_STATUS_OUT_OF_MEMORY;
    }

    o->frame_length = frame_length;

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

    ma_result result = MA_SUCCESS;
    if (object->is_started) {
        if (ma_device_is_started(&(object->device))) {
            return PV_RECORDER_STATUS_SUCCESS;
        } else {
            ma_device_uninit(&(object->device));
            object->is_started = false;
        }
    }

    if (ma_device_get_state(&(object->device)) == ma_device_state_uninitialized) {
        result = ma_device_init(&(object->context), &(object->device_config), &(object->device));
        if (result != MA_SUCCESS) {
            return ma_result_to_pv_recorder_status(result);
        }
    }

    result = ma_device_start(&(object->device));
    if (result != MA_SUCCESS) {
        return ma_result_to_pv_recorder_status(result);
    }

    object->is_started = true;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API pv_recorder_status_t pv_recorder_stop(pv_recorder_t *object) {
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    ma_mutex_lock(&object->mutex);
    pv_circular_buffer_reset(object->buffer);
    ma_mutex_unlock(&object->mutex);

    if (!ma_device_is_started(&(object->device))) {
        return PV_RECORDER_STATUS_SUCCESS;
    }

    ma_result result = ma_device_stop(&(object->device));
    if (result != MA_SUCCESS) {
        return ma_result_to_pv_recorder_status(result);
    }

    object->is_started = false;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API pv_recorder_status_t pv_recorder_read(pv_recorder_t *object, int16_t *frame) {
    if (!object) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!frame) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!ma_device_is_started(&object->device)) {
        return PV_RECORDER_STATUS_INVALID_STATE;
    }

    int16_t *read_ptr = frame;
    int32_t processed = 0;
    int32_t remaining = object->frame_length;

    for (int32_t i = 0; i < READ_RETRY_COUNT; i++) {
        ma_mutex_lock(&object->mutex);

        if (!ma_device_is_started(&object->device)) {
            ma_mutex_unlock(&object->mutex);
            return PV_RECORDER_STATUS_SUCCESS;
        }

        const int32_t length = pv_circular_buffer_read(object->buffer, read_ptr, remaining);
        processed += length;

        if (processed == object->frame_length) {
            ma_mutex_unlock(&object->mutex);

            if (object->is_debug_logging_enabled) {
                for (int32_t j = 0; j < object->frame_length; j++) {
                    if ((frame[j] > ABSOLUTE_SILENCE_THRESHOLD) || (frame[j] < -ABSOLUTE_SILENCE_THRESHOLD)) {
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

PV_API void pv_recorder_set_debug_logging(
        pv_recorder_t *object,
        bool is_debug_logging_enabled) {
    if (!object) {
        return;
    }

    object->is_debug_logging_enabled = is_debug_logging_enabled;
}

PV_API bool pv_recorder_get_is_recording(pv_recorder_t *object) {
    if (!object) {
        return false;
    }
    return ma_device_is_started(&object->device);
}

PV_API const char *pv_recorder_get_selected_device(pv_recorder_t *object) {
    if (!object) {
        return NULL;
    }
    return object->device.capture.name;
}

PV_API pv_recorder_status_t pv_recorder_get_available_devices(
        int32_t *device_list_length,
        char ***device_list) {
    if (!device_list_length) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }
    if (!device_list) {
        return PV_RECORDER_STATUS_INVALID_ARGUMENT;
    }

    ma_context context;
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        return ma_result_to_pv_recorder_status(result);
    }

    ma_device_info *capture_info;
    ma_uint32 capture_count;
    result = ma_context_get_devices(
            &context,
            NULL,
            NULL,
            &capture_info,
            &capture_count);
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

    *device_list_length = (int32_t) capture_count;
    *device_list = d;

    return PV_RECORDER_STATUS_SUCCESS;
}

PV_API void pv_recorder_free_available_devices(
        int32_t device_list_length,
        char **device_list) {
    if (device_list && (device_list_length > 0)) {
        for (int32_t i = 0; i < device_list_length; i++) {
            free(device_list[i]);
        }
        free(device_list);
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

PV_API int32_t pv_recorder_sample_rate(void) {
    return PV_RECORDER_SAMPLE_RATE;
}

PV_API const char *pv_recorder_version(void) {
    return PV_RECORDER_VERSION;
}
