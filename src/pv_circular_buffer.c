/*
    Copyright 2021-2022 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdlib.h>
#include <string.h>

#include "pv_circular_buffer.h"

struct pv_circular_buffer {
    void *buffer;
    int32_t capacity;
    int32_t count;
    int32_t element_size;
    int32_t read_index;
    int32_t write_index;
};

pv_circular_buffer_status_t pv_circular_buffer_init(
        int32_t capacity,
        int32_t element_size,
        pv_circular_buffer_t **object) {
    if (capacity <= 0) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (element_size <= 0) {
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

    o->buffer = malloc(capacity * element_size);
    if (!(o->buffer)) {
        pv_circular_buffer_delete(o);
        return PV_CIRCULAR_BUFFER_STATUS_OUT_OF_MEMORY;
    }

    o->capacity = capacity;
    o->element_size = element_size;

    *object = o;

    return PV_CIRCULAR_BUFFER_STATUS_SUCCESS;
}

void pv_circular_buffer_delete(pv_circular_buffer_t *object) {
    if (object) {
        free(object->buffer);
        free(object);
    }
}

int32_t pv_circular_buffer_read(pv_circular_buffer_t *object, void *buffer, int32_t length) {
    if (!object) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!buffer) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!length) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if ((length <= 0) || (length > object->capacity)) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }

    void *dst_ptr = buffer;
    const void *src_ptr = (char *) object->buffer + (object->read_index * object->element_size);

    const int32_t available = object->capacity - object->read_index;
    const int32_t max_copy = (object->count < length) ? object->count : length;
    const int32_t to_copy = (max_copy < available) ? max_copy : available;

    memcpy(dst_ptr, src_ptr, to_copy * object->element_size);

    object->read_index = (object->read_index + to_copy) % object->capacity;

    const int32_t remaining = max_copy - to_copy;
    if (remaining > 0) {
        dst_ptr = (char *) buffer + (to_copy * object->element_size);
        src_ptr = object->buffer;

        memcpy(dst_ptr, src_ptr, remaining * object->element_size);

        object->read_index = remaining;
    }

    object->count -= max_copy;

    return max_copy;
}

pv_circular_buffer_status_t pv_circular_buffer_write(pv_circular_buffer_t *object, const void *buffer, int32_t length) {
    if (!object) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if (!buffer) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }
    if ((length <= 0) || (length > object->capacity)) {
        return PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT;
    }

    pv_circular_buffer_status_t status = PV_CIRCULAR_BUFFER_STATUS_SUCCESS;

    void *dst_ptr = (char *) object->buffer + (object->write_index * object->element_size);
    const void *src_ptr = buffer;

    const int32_t available = object->capacity - object->write_index;
    const int32_t to_copy = (length < available) ? length : available;

    memcpy(dst_ptr, src_ptr, to_copy * object->element_size);

    object->write_index = (object->write_index + to_copy) % object->capacity;
    object->count += to_copy;

    const int32_t remaining = length - to_copy;
    if (remaining > 0) {
        dst_ptr = object->buffer;
        src_ptr = (char *) buffer + (to_copy * object->element_size);

        memcpy(dst_ptr, src_ptr, remaining * object->element_size);

        object->write_index = remaining;
        object->count += remaining;
    }

    if(object->count > object->capacity) {
        status = PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW;
        object->count = object->capacity;
        object->read_index = (object->write_index + 1) % object->capacity;
    }

    return status;
}

void pv_circular_buffer_reset(pv_circular_buffer_t *object) {
    object->count = 0;
    object->read_index = 0;
    object->write_index = 0;
}

const char *pv_circular_buffer_status_to_string(pv_circular_buffer_status_t status) {
    static const char *const STRINGS[] = {
            "SUCCESS",
            "OUT_OF_MEMORY",
            "INVALID_ARGUMENT",
            "WRITE_OVERFLOW"};

    int32_t size = sizeof(STRINGS) / sizeof(STRINGS[0]);
    if (status < PV_CIRCULAR_BUFFER_STATUS_SUCCESS || status >= (PV_CIRCULAR_BUFFER_STATUS_SUCCESS + size)) {
        return NULL;
    }

    return STRINGS[status - PV_CIRCULAR_BUFFER_STATUS_SUCCESS];
}
