/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "pv_circular_buffer.h"

struct pv_circular_buffer {
    void *buffer;
    void *buffer_end;
    int32_t capacity;
    int32_t count;
    int32_t elem_size;
    void *read_ptr;
    void *write_ptr;
    int32_t read_tries;
    int32_t read_sleep_useconds;
    pthread_mutex_t mutex;
};

pv_circular_buffer_status_t pv_circular_buffer_init(
        int32_t capacity,
        int32_t elem_size,
        int32_t read_tries,
        int32_t read_sleep_micro_seconds,
        pv_circular_buffer_t **object) {
    if (capacity <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (elem_size <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (read_tries <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (read_sleep_micro_seconds <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!object) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }

    *object = NULL;

    pv_circular_buffer_t *o = calloc(1, sizeof(pv_circular_buffer_t));
    if (!o) {
        return PV_CIRCULAR_BUFFER_STATUS_OUT_OF_MEMORY;
    }

    o->buffer = malloc(capacity * sizeof(int16_t));
    if (!(o->buffer)) {
        free(o);
        return PV_CIRCULAR_BUFFER_STATUS_OUT_OF_MEMORY;
    }

    o->capacity = capacity;
    o->elem_size = elem_size;
    o->count = 0;

    o->buffer_end = ((char *)o->buffer + (capacity * elem_size));
    o->read_ptr = o->buffer;
    o->write_ptr = o->buffer;

    o->read_tries = read_tries;
    o->read_sleep_useconds = read_sleep_micro_seconds * 1000;

    *object = o;

    return PV_CIRCULAR_BUFFER_STATUS_SUCCESS;
}

void pv_circular_buffer_delete(pv_circular_buffer_t *object) {
    if (object) {
        free(object->buffer);
        free(object);
    }
}

pv_circular_buffer_status_t pv_circular_buffer_read(pv_circular_buffer_t *object, void *buffer, int32_t *length) {
    if (!object) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!buffer) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!length) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (*length <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }

    void *buffer_ptr = buffer;
    int32_t processed = 0, tries = 0;
    while (processed < *length) {
        pthread_mutex_lock(&object->mutex);

        if (object->count == 0) {
            pthread_mutex_unlock(&object->mutex);
            if (tries == object->read_tries) {
                *length = processed;
                return PV_CIRCULAR_BUFFER_STATUS_READ_TIMEOUT;
            }
            tries++;
            usleep(object->read_sleep_useconds);
            continue;
        }

        memcpy(buffer_ptr, object->read_ptr, object->elem_size);
        buffer_ptr = (char *)buffer_ptr + object->elem_size;
        object->read_ptr = (char *)object->read_ptr + object->elem_size;
        if (object->read_ptr == object->buffer_end) {
            object->read_ptr = object->buffer;
        }
        object->count--;
        processed++;

        pthread_mutex_unlock(&object->mutex);
    }

    return PV_CIRCULAR_BUFFER_STATUS_SUCCESS;
}

pv_circular_buffer_status_t pv_circular_buffer_write(pv_circular_buffer_t *object, const void *buffer, int32_t length) {
    if (!object) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!buffer) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (length <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }

    const void *buffer_ptr = buffer;
    pv_circular_buffer_status_t status = PV_CIRCULAR_BUFFER_STATUS_SUCCESS;
    for (int32_t i = 0; i < length; i++) {
        pthread_mutex_lock(&object->mutex);

        if (object->count == object->capacity) {
            status = PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW;
            object->count--;
        }

        memcpy(object->write_ptr, buffer_ptr, object->elem_size);
        buffer_ptr = (char *)buffer_ptr + object->elem_size;
        object->write_ptr = (char *)object->write_ptr + object->elem_size;
        if (object->write_ptr == object->buffer_end) {
            object->write_ptr = object->buffer;
        }
        object->count++;

        pthread_mutex_unlock(&object->mutex);
    }

    return status;
}

void pv_circular_buffer_reset(pv_circular_buffer_t *object) {
    object->count = 0;
    object->read_ptr = object->buffer;
    object->write_ptr = object->buffer;
}

const char *pv_circular_buffer_status_to_string(pv_circular_buffer_status_t status) {
    static const char *const STRINGS[] = {
            "SUCCESS",
            "OUT_OF_MEMORY",
            "INVALID_ARGUMENT",
            "READ_TIMEOUT",
            "WRITE_OVERFLOW"};

    int32_t size = sizeof(STRINGS)/sizeof(STRINGS[0]);
    if (status < PV_CIRCULAR_BUFFER_STATUS_SUCCESS || status >= (PV_CIRCULAR_BUFFER_STATUS_SUCCESS + size)) {
        return NULL;
    }

    return STRINGS[status - PV_CIRCULAR_BUFFER_STATUS_SUCCESS];
}
