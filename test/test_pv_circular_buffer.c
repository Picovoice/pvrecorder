/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pv_circular_buffer.h"

#define CB_ASSERT(condition, message)                                                           \
    if ((condition) == 0) {                                                                     \
        fprintf(stderr, "%s:%s():at_line %d: %s", __FILE__, __FUNCTION__, __LINE__, message);   \
        exit(1);                                                                                \
    }                                                                                           \
    (void)(0)

static void test_pv_circular_buffer_once(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(128, sizeof(int16_t), &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70};
    int32_t in_size = sizeof(in_buffer) / sizeof(in_buffer[0]);

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to write buffer.");

    status = pv_circular_buffer_read(cb, out_buffer, &out_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to read buffer.");

    CB_ASSERT(in_size == out_size, "Read and write buffers have different sizes.");

    for (int32_t i = 0; i < in_size; i++) {
        CB_ASSERT(in_buffer[i] == out_buffer[i], "Read and write buffers have different values.");
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_empty(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(128, sizeof(int16_t), &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int32_t out_size = 5;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));

    status = pv_circular_buffer_read(cb, out_buffer, &out_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_READ_EMPTY, "Expected a read time out.");
    CB_ASSERT(out_size == 0, "Expected buffer size to be 0.");

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_write_overflow(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            8,
            sizeof(int16_t),
            &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70, 100, 0, 1, -100};
    int32_t in_size = sizeof(in_buffer) / sizeof(in_buffer[0]);

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW, "Expected write overflow.");

    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_write(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(2048, sizeof(int16_t), &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int32_t in_size = 512;
    int16_t in_buffer[in_size];
    for (int32_t i = 0; i < in_size; i++) {
        in_buffer[i] = (int16_t) ((rand() % (2000 + 1)) - 1000);
    }

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));

    for (int32_t i = 0; i < 10; i++) {
        pv_circular_buffer_write(cb, in_buffer, in_size);
        pv_circular_buffer_read(cb, out_buffer, &out_size);
        CB_ASSERT(in_size == out_size, "Read and write buffers have different sizes.");
        for (int32_t j = 0; j < in_size; j++) {
            CB_ASSERT(in_buffer[i] == out_buffer[i], "Read and write buffers have different values.");
        }
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

int main() {
    srand(time(NULL));

    test_pv_circular_buffer_once();
    test_pv_circular_buffer_read_empty();
    test_pv_circular_buffer_write_overflow();
    test_pv_circular_buffer_read_write();

    return 0;
}