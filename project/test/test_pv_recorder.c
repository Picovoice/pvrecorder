/*
    Copyright 2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include "string.h"

#include "pv_recorder.h"
#include "test_helper.h"

static void init_test_helper(
        int32_t frame_length,
        int32_t device_index,
        int32_t buffered_frames_count,
        pv_recorder_status_t expected_status) {
    pv_recorder_t *recorder = NULL;
    pv_recorder_status_t status;

    status = pv_recorder_init(frame_length, device_index, buffered_frames_count, &recorder);

    check_condition(
            status == expected_status,
            __FUNCTION__,
            __LINE__,
            "Recorder initialization returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(expected_status));
    if (recorder) {
        pv_recorder_delete(recorder);
    }
}

static void test_pv_recorder_init(void) {
    printf("Initialize with valid parameters\n");
    init_test_helper(512, 0, 10, PV_RECORDER_STATUS_SUCCESS);

    printf("Initialize with invalid device index (negative)\n");
    init_test_helper(512, -2, 10, PV_RECORDER_STATUS_INVALID_ARGUMENT);

    printf("Initialize with invalid device index (too high)\n");
    init_test_helper(512, 500, 10, PV_RECORDER_STATUS_INVALID_ARGUMENT);

    printf("Initialize with invalid frame length\n");
    init_test_helper(-1, 0, 10, PV_RECORDER_STATUS_INVALID_ARGUMENT);

    printf("Initialize with invalid buffered frames count\n");
    init_test_helper(512, 0, 0, PV_RECORDER_STATUS_INVALID_ARGUMENT);

    printf("Initialize with null recorder pointer\n");
    pv_recorder_status_t status = pv_recorder_init(512, 0, 10, NULL);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "Recorder initialization returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));
}

static void test_pv_recorder_start_stop(void) {
    pv_recorder_t *recorder = NULL;
    pv_recorder_status_t status;
    int16_t frame[512];

    status = pv_recorder_init(512, 0, 10, &recorder);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder initialization returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    printf("Check is_recording on NULL\n");
    bool is_recording = pv_recorder_get_is_recording(NULL);
    check_condition(
            is_recording == false,
            __FUNCTION__,
            __LINE__,
            "get_is_recording returned true on a NULL object.");

    printf("Check is_recording on before start\n");
    is_recording = pv_recorder_get_is_recording(recorder);
    check_condition(
            is_recording == false,
            __FUNCTION__,
            __LINE__,
            "get_is_recording returned true - expected false.");

    printf("Call start on null object\n");
    status = pv_recorder_start(NULL);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "Recorder start returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    printf("Call read before start object\n");
    status = pv_recorder_read(recorder, frame);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_STATE,
            __FUNCTION__,
            __LINE__,
            "Recorder read returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_STATE));

    printf("Call start on valid recorder object\n");
    status = pv_recorder_start(recorder);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder start returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    printf("Call read on null recorder\n");
    status = pv_recorder_read(NULL, frame);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "Recorder read returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    printf("Call read with null frame\n");
    status = pv_recorder_read(recorder, NULL);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "Recorder read returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    printf("Call read with valid args\n");
    status = pv_recorder_read(recorder, frame);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder read returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    printf("Check is_recording on started recorder\n");
    is_recording = pv_recorder_get_is_recording(recorder);
    check_condition(
            is_recording == true,
            __FUNCTION__,
            __LINE__,
            "get_is_recording returned false - expected true.");

    printf("Call stop on null recorder object\n");
    status = pv_recorder_stop(NULL);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "Recorder stop returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    printf("Call stop on valid recorder object\n");
    status = pv_recorder_stop(recorder);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder stop returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    printf("Check is_recording on stopped recorder\n");
    is_recording = pv_recorder_get_is_recording(recorder);
    check_condition(
            is_recording == false,
            __FUNCTION__,
            __LINE__,
            "get_is_recording returned true - expected false.");

    pv_recorder_delete(recorder);
}

static void test_pv_recorder_set_debug_logging(void) {
    pv_recorder_t *recorder = NULL;
    pv_recorder_status_t status = pv_recorder_init(512, 0, 10, &recorder);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder initialization returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    pv_recorder_set_debug_logging(NULL, true);
    pv_recorder_set_debug_logging(recorder, true);

    pv_recorder_delete(recorder);
}

static void test_pv_recorder_get_selected_device(void) {
    pv_recorder_t *recorder = NULL;
    pv_recorder_status_t status = pv_recorder_init(512, 0, 10, &recorder);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "Recorder initialization returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));

    check_condition(
            pv_recorder_get_selected_device(NULL) == NULL,
            __FUNCTION__,
            __LINE__,
            "pv_recorder_get_selected_device should have returned NULL.");

    check_condition(
            strcmp(pv_recorder_get_selected_device(recorder), "") != 0,
            __FUNCTION__,
            __LINE__,
            "pv_recorder_get_selected_device should have returned a device name");

    pv_recorder_delete(recorder);
}

static void test_pv_recorder_get_available_devices(void) {
    pv_recorder_status_t status;
    int32_t device_list_length = -1;
    char **device_list = NULL;

    status = pv_recorder_get_available_devices(NULL, &device_list);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "pv_recorder_get_available_devices returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    status = pv_recorder_get_available_devices(&device_list_length, NULL);
    check_condition(
            status == PV_RECORDER_STATUS_INVALID_ARGUMENT,
            __FUNCTION__,
            __LINE__,
            "pv_recorder_get_available_devices returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT));

    status = pv_recorder_get_available_devices(&device_list_length, &device_list);
    check_condition(
            status == PV_RECORDER_STATUS_SUCCESS,
            __FUNCTION__,
            __LINE__,
            "pv_recorder_get_available_devices returned %s - expected %s.",
            pv_recorder_status_to_string(status),
            pv_recorder_status_to_string(PV_RECORDER_STATUS_SUCCESS));
    check_condition(
            device_list_length >= 0,
            __FUNCTION__,
            __LINE__,
            "device_list_length should have been greater than 0, instead got %d",
            device_list_length);
    check_condition(
            device_list != NULL,
            __FUNCTION__,
            __LINE__,
            "device_list should have not been NULL");

    pv_recorder_free_available_devices(device_list_length, device_list);
}

static void test_pv_recorder_sample_rate(void) {
    int32_t sample_rate = pv_recorder_sample_rate();
    check_condition(
            sample_rate > 0,
            __FUNCTION__,
            __LINE__,
            "Sample rate was invalid (%d).",
            sample_rate);
}

static void test_pv_recorder_version(void) {
    const char *version = pv_recorder_version();
    check_condition(
            strcmp(version, "") != 0,
            __FUNCTION__,
            __LINE__,
            "Version was supposed to be a non-empty string.");
}

int main() {
    srand(time(NULL));
    test_pv_recorder_get_available_devices();
    test_pv_recorder_sample_rate();
    test_pv_recorder_version();
    test_pv_recorder_init();
    test_pv_recorder_start_stop();
    test_pv_recorder_set_debug_logging();
    test_pv_recorder_get_selected_device();
    return 0;
}