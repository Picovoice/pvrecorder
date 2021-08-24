/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#ifndef PV_CIRCULAR_BUFFER_H
#define PV_CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Forward declaration of PV_circular_buffer object. It deals with asynchronous calls, handles reading and writing to a buffer.
 */
typedef struct pv_circular_buffer pv_circular_buffer_t;

/**
 * Status codes.
 */
typedef enum {
    PV_CIRCULAR_BUFFER_STATUS_SUCCESS = 0,
    PV_CIRCULAR_BUFFER_STATUS_OUT_OF_MEMORY,
    PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT,
    PV_CIRCULAR_BUFFER_STATUS_READ_TIMEOUT,
    PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW,
} pv_circular_buffer_status_t;

/**
 * Constructor for PV_circular_buffer object.
 *
 * @param capacity Capacity of the buffer to read and write.
 * @param elem_size Size of each item in the buffer.
 * @param read_tries Number of tries to do before a read call fails.
 * @param read_sleep_micro_seconds Number in micro seconds to wait if there is no buffer items to be read.
 * @param object[out] Circular buffer object.
 * @return Status Code. Returns PV_CIRCULAR_BUFFER_STATUS_OUT_OF_MEMORY or PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT
 * on failure.
 */
pv_circular_buffer_status_t pv_circular_buffer_init(
        int32_t capacity,
        int32_t elem_size,
        int32_t read_tries,
        int32_t read_sleep_micro_seconds,
        pv_circular_buffer_t **object);

/**
 * Destructor for PV_circular_buffer object.
 *
 * @param object Circular buffer object.
 */
void pv_circular_buffer_delete(pv_circular_buffer_t *object);

/**
 * Reads and copies the elements to the provided param ${buffer}. The param ${length} contains a new length
 * if a status other that PV_CIRCULAR_BUFFER_STATUS_SUCCESS is returned.
 *
 * @param object Circular buffer object.
 * @param buffer[out] A pointer to copy the elements into.
 * @param length[in,out] The amount to copy to the buffer. Replaced to the actual amount of length copied if status
 * is not PV_CIRCULAR_BUFFER_STATUS_SUCCESS.
 * @return Status Code. Returns PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT or PV_CIRCULAR_BUFFER_STATUS_READ_TIMEOUT on failure.
 */
pv_circular_buffer_status_t pv_circular_buffer_read(pv_circular_buffer_t *object, void *buffer, int32_t *length);

/**
 * Writes and copies the elements of param ${buffer} to the object's buffer. Overwrites existing frames if the buffer
 * is full and returns PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW which is not a failure.
 *
 * @param object Circular buffer object.
 * @param buffer A pointer to copy its elements to the object's buffer.
 * @param length The amount of elements to copy.
 * @return Status Code. Returns PV_CIRCULAR_BUFFER_STATUS_INVALID_ARGUMENT on failure.
 */
pv_circular_buffer_status_t pv_circular_buffer_write(pv_circular_buffer_t *object, const void *buffer, int32_t length);

void pv_circular_buffer_reset(pv_circular_buffer_t *object);

/**
 * Provides string representations of status codes.
 *
 * @param status Status code.
 * @return String representation.
 */
const char *pv_circular_buffer_status_to_string(pv_circular_buffer_status_t status);

#endif //PV_CIRCULAR_BUFFER_H