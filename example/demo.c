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
#include <signal.h>

#include "pv_recorder.h"

static volatile bool is_interrupted = false;

void interrupt_handler(int _) {
    (void) _;
    is_interrupted = true;
}

int main() {
    signal(SIGINT, interrupt_handler);

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
    status = pv_recorder_init(-1, 4096, &recorder);
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

    int16_t *pcm = malloc(512 * sizeof(int16_t));
    int32_t length = 512;
    while (!is_interrupted) {
        status = pv_recorder_read(recorder, pcm, &length);
        if (status != PV_RECORDER_STATUS_SUCCESS) {
            fprintf(stdout, "Failed to read with %s.\n", pv_recorder_status_to_string(status));
            exit(-1);
        }
    }

    fprintf(stdout, "Stop recording...\n");
    status = pv_recorder_stop(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stdout, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Deleting pv_recorder...\n");
    pv_recorder_delete(recorder);

    return 0;
}