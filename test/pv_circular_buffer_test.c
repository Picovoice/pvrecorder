/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "pv_circular_buffer.h"

#define CB_ASSERT(condition, message) \
    assert(( (void)(message), (condition) ))


typedef struct {
    pv_circular_buffer_t *cb;
    int16_t *buffer;
    int32_t size;
} args_t;

static void *read_func(void *args) {
    args_t *out_args = (args_t *) args;
    pv_circular_buffer_status_t status = pv_circular_buffer_read(out_args->cb, out_args->buffer, &out_args->size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to read buffer.");
    return NULL;
}

static void *write_func(void *args) {
    args_t *in_args = (args_t *) args;
    pv_circular_buffer_status_t status = pv_circular_buffer_write(in_args->cb, in_args->buffer, in_args->size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to write buffer.");
    return NULL;
}

static void test_pv_circular_buffer_once(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            128,
            sizeof(int16_t),
            100,
            10,
            &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70};
    int32_t in_size = sizeof(in_buffer)/sizeof(in_buffer[0]);

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

static void test_pv_circular_buffer_read_timeout(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            128,
            sizeof(int16_t),
            100,
            10,
            &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int32_t out_size = 5;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));

    status = pv_circular_buffer_read(cb, out_buffer, &out_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_READ_TIMEOUT, "Expected a read time out.");
    CB_ASSERT(out_size == 0, "Expected buffer size to be 0.");

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_write_overflow(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            8,
            sizeof(int16_t),
            100,
            10,
            &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70, 100, 0, 1, -100};
    int32_t in_size = sizeof(in_buffer)/sizeof(in_buffer[0]);

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW, "Expected write overflow.");

    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_write(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            3072,
            sizeof(int16_t),
            100,
            1,
            &cb);
    CB_ASSERT(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, "Failed to initialize buffer.");

    int32_t in_size = 4096;
    int16_t in_buffer[in_size];
    for (int32_t i = 0; i < in_size; i++) {
        in_buffer[in_size] = (int16_t) ((rand() % (2000 + 1)) - 2000);
    }

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));

    args_t in_args = {cb, in_buffer, in_size};
    args_t out_args = {cb, out_buffer, out_size};

    pthread_t read_thread, write_thread;
    pthread_create(&write_thread, NULL, write_func, (void *) &in_args);
    pthread_create(&read_thread, NULL, read_func, (void *) &out_args);
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);

    printf("%d %d\n", in_args.size, out_args.size);
    CB_ASSERT(in_args.size == out_args.size, "Read and write buffers have different sizes.");
    for (int32_t i = 0; i < in_size; i++) {
        CB_ASSERT(in_args.buffer[i] == out_args.buffer[i], "Read and write buffers have different values.");
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

int main() {
    srand(time(NULL));

    test_pv_circular_buffer_once();
    test_pv_circular_buffer_read_timeout();
    test_pv_circular_buffer_write_overflow();
    test_pv_circular_buffer_read_write();

    return 0;
}