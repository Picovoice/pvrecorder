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
#include <unistd.h>

#include "pv_recorder.h"

static void pv_recorder_callback(const int16_t *pcm) {
    // do something with pcm
    (void) pcm;
}

struct extra_data {
    int a;
};

static void pv_recorder_callback_with_data(const int16_t *pcm, void *user_data) {
    // use user data
    struct extra_data *data = (struct extra_data *) user_data;
    (void) data;
    // do something with pcm
    (void) pcm;
}

int main() {
    char **devices;
    int32_t count;

    // List devices
    pv_recorder_status_t status = pv_recorder_get_audio_devices(&count, &devices);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to get audio devices with: %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Printing devices...\n");
    for (int32_t i = 0; i < count; i++) {
        fprintf(stdout, "index: %d, name: %s\n", i, devices[i]);
    }
    fprintf(stdout, "\n");

    pv_recorder_free_device_list(count, devices);
    devices = NULL;

    // Use PV_Recorder
    fprintf(stdout, "Initializing pv_recorder...\n");

    pv_recorder_t *recorder;
    status = pv_recorder_init(-1, 512, pv_recorder_callback, &recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to initialize device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Start recording...\n");
    status = pv_recorder_start(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Sleeping for 1 second...\n");

    sleep(1);

    fprintf(stdout, "Stop recording...\n");
    status = pv_recorder_stop(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Deleting pv_recorder...\n");
    pv_recorder_delete(recorder);

    struct extra_data data = {1};

    fprintf(stdout, "Initializing with data...\n");
    status = pv_recorder_init_with_data(-1, 512, pv_recorder_callback_with_data, &data, &recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to initialize device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Start recording...\n");
    status = pv_recorder_start(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Sleeping for 1 second...\n");

    sleep(1);

    fprintf(stdout, "Deleting pv_recorder...\n");
    pv_recorder_delete(recorder);

    return 0;
}