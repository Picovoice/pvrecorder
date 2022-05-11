/*
    Copyright 2021-2022 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pv_circular_buffer.h"

static char error_message[256] = {0};

static const char *test_error_message(const char *message, va_list args) {
    vsnprintf(error_message, sizeof(error_message) / sizeof (error_message[0]), message, args);
    return error_message;
}

static void check_condition(bool condition, const char *function, int32_t line, const char *message, ...) {
    if (condition == 0) {
        va_list args;
        va_start(args, message);
        fprintf(stderr, "%s:%s():at_line %d: %s", __FILE__, function, line, test_error_message(message, args));
        va_end(args);
        exit(1);
    }
}

static void test_pv_circular_buffer_once(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(128, sizeof(int16_t), &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70};
    int32_t in_size = sizeof(in_buffer) / sizeof(in_buffer[0]);

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));
    check_condition(out_buffer != NULL, __FUNCTION__, __LINE__, "Failed to allocate memory.");

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to write buffer.");

    int32_t length = pv_circular_buffer_read(cb, out_buffer, out_size);
    check_condition(length == out_size, __FUNCTION__ , __LINE__, "Read and write buffers have different sizes.");

    for (int32_t i = 0; i < in_size; i++) {
        check_condition(in_buffer[i] == out_buffer[i],
                        __FUNCTION__ ,
                        __LINE__,
                        "Read and write buffers have different values at index %d with values: in_buffer: %d, out_buffer: %d",
                        i,
                        in_buffer[i],
                        out_buffer[i]);
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_incomplete(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(128, sizeof(int16_t), &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int32_t out_size = 5;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));
    check_condition(out_buffer != NULL, __FUNCTION__, __LINE__, "Failed to allocate memory.");

    int32_t length = pv_circular_buffer_read(cb, out_buffer, out_size);
    check_condition(length == 0, __FUNCTION__ , __LINE__, "Expected buffer size to be 0.");

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_write_overflow(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(
            10,
            sizeof(int16_t),
            &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int16_t in_buffer[] = {5, 7, -20, 35, 70, 100, 0, 1, -100};
    int32_t in_size = sizeof(in_buffer) / sizeof(in_buffer[0]);

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to write to buffer.");

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_WRITE_OVERFLOW, __FUNCTION__ , __LINE__, "Expected write overflow.");

    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_write(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(2048, sizeof(int16_t), &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int32_t in_size = 512;
    int16_t in_buffer[in_size];
    for (int32_t i = 0; i < in_size; i++) {
        in_buffer[i] = (int16_t) ((rand() % (2000 + 1)) - 1000);
    }

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));
    check_condition(out_buffer != NULL, __FUNCTION__, __LINE__, "Failed to allocate memory.");

    for (int32_t i = 0; i < 10; i++) {
        pv_circular_buffer_write(cb, in_buffer, in_size);
        int32_t length = pv_circular_buffer_read(cb, out_buffer, out_size);
        check_condition(length == out_size, __FUNCTION__ , __LINE__, "Read buffer received different sizes.");
        for (int32_t j = 0; j < in_size; j++) {
            check_condition(in_buffer[i] == out_buffer[i],
                            __FUNCTION__ ,
                            __LINE__,
                            "Read and write buffers have different values at index %d with values: in_buffer: %d, out_buffer: %d",
                            i,
                            in_buffer[i],
                            out_buffer[i]);
        }
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_read_write_one_by_one(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(12, sizeof(int16_t), &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int32_t in_size = 64;
    int16_t in_buffer[in_size];
    for (int32_t i = 0; i < in_size; i++) {
        in_buffer[i] = (int16_t) ((rand() % (2000 + 1)) - 1000);
    }

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));
    check_condition(out_buffer != NULL, __FUNCTION__, __LINE__, "Failed to allocate memory.");

    for (int32_t i = 0; i < in_size; i++) {
        status = pv_circular_buffer_write(cb, in_buffer + i, 1);
        check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to write to buffer.");

        int32_t length = pv_circular_buffer_read(cb, out_buffer + i, 1);
        check_condition(length == 1, __FUNCTION__ , __LINE__, "Buffer read received incorrect output length.");

        check_condition(in_buffer[i] == out_buffer[i], __FUNCTION__ , __LINE__, "Buffer have incorrect sizes.");
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

static void test_pv_circular_buffer_zeros(void) {
    pv_circular_buffer_t *cb;
    pv_circular_buffer_status_t status = pv_circular_buffer_init(100, sizeof(int16_t), &cb);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to initialize buffer.");

    int32_t in_size = 100;
    int16_t in_buffer[in_size];
    for (int32_t i = 0; i < in_size; i++) {
        in_buffer[i] = 0;
    }

    int32_t out_size = in_size;
    int16_t *out_buffer = malloc(out_size * sizeof(int16_t));
    check_condition(out_buffer != NULL, __FUNCTION__, __LINE__, "Failed to allocate memory.");

    for (int32_t i = 0; i < in_size; i++) {
        out_buffer[i] = (int16_t) 9999;
    }

    status = pv_circular_buffer_write(cb, in_buffer, 40);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to write to buffer.");

    int32_t length = pv_circular_buffer_read(cb, out_buffer, 40);
    check_condition(length == 40, __FUNCTION__ , __LINE__, "Buffer read received incorrect output length.");

    for (int32_t i = 0; i < 40; i++) {
        check_condition(out_buffer[i] == 0, __FUNCTION__ , __LINE__, "Buffer have incorrect values at %d.", i);
    }

    status = pv_circular_buffer_write(cb, in_buffer, in_size);
    check_condition(status == PV_CIRCULAR_BUFFER_STATUS_SUCCESS, __FUNCTION__ , __LINE__, "Failed to write to buffer.");

    length = pv_circular_buffer_read(cb, out_buffer, out_size);
    check_condition(length == out_size, __FUNCTION__ , __LINE__, "Buffer read received incorrect output length.");

    for (int32_t i = 0; i < out_size; i++) {
        check_condition(out_buffer[i] == 0, __FUNCTION__ , __LINE__, "Buffer have incorrect values at %d.", i);
    }

    free(out_buffer);
    pv_circular_buffer_delete(cb);
}

int main() {
    srand(time(NULL));

    test_pv_circular_buffer_once();
    test_pv_circular_buffer_read_incomplete();
    test_pv_circular_buffer_write_overflow();
    test_pv_circular_buffer_read_write();
    test_pv_circular_buffer_read_write_one_by_one();
    test_pv_circular_buffer_zeros();

    return 0;
}