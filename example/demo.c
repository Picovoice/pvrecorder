/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pv_recorder.h"

static volatile bool is_interrupted = false;

void interrupt_handler(int _) {
    (void) _;
    is_interrupted = true;
}

static void print_usage(const char *program) {
    fprintf(stderr, "usage: %s --show_audio_devices\n"
                    "       %s audio_device_index path_to_raw_file\n", program, program);
}

int main(int argc, char *argv[]) {
    if ((argc != 2) && (argc != 3)) {
        print_usage(argv[0]);
        exit(1);
    }

    signal(SIGINT, interrupt_handler);

    if (strcmp(argv[1], "--show_audio_devices") == 0) {
        char **devices;
        int32_t count;

        // List devices
        pv_recorder_status_t status = pv_recorder_get_audio_devices(&count, &devices);
        if (status != PV_RECORDER_STATUS_SUCCESS) {
            fprintf(stderr, "Failed to get audio devices with: %s.\n", pv_recorder_status_to_string(status));
            exit(-1);
        }

        fprintf(stdout, "Printing devices...\n");
        for (int32_t i = 0; i < count; i++) {
            fprintf(stderr, "index: %d, name: %s\n", i, devices[i]);
        }

        pv_recorder_free_device_list(count, devices);
        return 0;
    }

    const int32_t device_index = (int32_t) strtol(argv[1], NULL, 10);
    const char *path_to_raw_file = argv[2];

    // Use PV_Recorder
    fprintf(stdout, "Initializing pv_recorder...\n");

    pv_recorder_t *recorder;
    pv_recorder_status_t status = pv_recorder_init(device_index, 512, 100, true, &recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to initialize device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    const char *selected_device = pv_recorder_get_selected_device(recorder);
    fprintf(stdout, "Selected device: %s.\n", selected_device);

    fprintf(stdout, "Start recording...\n");
    status = pv_recorder_start(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    int16_t *pcm = malloc(512 * sizeof(int16_t));

    FILE *file = NULL;
    if (path_to_raw_file != NULL) {
        file = fopen(path_to_raw_file, "wb");
        if (file == NULL) {
            fprintf(stdout, "Failed to open file.\n");
        }
    }

    while (!is_interrupted) {
        status = pv_recorder_read(recorder, pcm);
        if (status != PV_RECORDER_STATUS_SUCCESS) {
            fprintf(stderr, "Failed to read with %s.\n", pv_recorder_status_to_string(status));
            exit(-1);
        }
        if (file != NULL) {
            fwrite(pcm, sizeof(int16_t), 512, file);
        }
    }

    if (file != NULL) {
        fclose(file);
    }

    fprintf(stdout, "Stop recording...\n");
    status = pv_recorder_stop(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(-1);
    }

    fprintf(stdout, "Deleting pv_recorder...\n");
    pv_recorder_delete(recorder);

    return 0;
}