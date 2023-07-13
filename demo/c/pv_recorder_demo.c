/*
    Copyright 2021-2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pv_recorder.h"

static volatile bool is_interrupted = false;

void interrupt_handler(int _) {
    (void) _;
    is_interrupted = true;
}

static struct option long_options[] = {
        {"show_audio_devices", no_argument,       NULL, 's'},
        {"output_wav_path",    required_argument, NULL, 'o'},
        {"audio_device_index", required_argument, NULL, 'd'}
};

static void print_usage(const char *program_name) {
    fprintf(stderr,
            "Usage : %s -o OUTPUT_WAV_PATH [-d AUDIO_DEVICE_INDEX]\n"
            "        %s --show_audio_devices\n",
            program_name,
            program_name);
}

static void show_audio_devices(void) {
    char **device_list = NULL;
    int32_t device_list_length = 0;

    // List devices
    pv_recorder_status_t status = pv_recorder_get_available_devices(
            &device_list_length,
            &device_list);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to get audio devices with: %s.\n", pv_recorder_status_to_string(status));
        exit(1);
    }

    fprintf(stdout, "Printing devices...\n");
    for (int32_t i = 0; i < device_list_length; i++) {
        fprintf(stdout, "index: %d, name: %s\n", i, device_list[i]);
    }

    pv_recorder_free_available_devices(device_list_length, device_list);
}

static void write_wav_header(FILE *file, int32_t num_samples) {
    int32_t sample_rate = pv_recorder_sample_rate();
    const char *chunk_id = "RIFF";
    const char *format = "WAVE";
    const char *subchunk1_id = "fmt ";
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    uint16_t num_channels = 1;
    uint32_t byte_rate = sample_rate * num_channels * (sizeof(int16_t));
    uint16_t block_align = num_channels * (sizeof(int16_t));
    uint16_t bits_per_sample = 16;
    const char *subchunk2_id = "data";
    uint32_t subchunk2_size = num_samples * num_channels * (sizeof(int16_t));;
    uint32_t chunk_size = 36 + subchunk2_size;

    // Write WAV header
    fwrite(chunk_id, 4, 1, file);
    fwrite(&chunk_size, sizeof(chunk_size), 1, file);
    fwrite(format, 4, 1, file);
    fwrite(subchunk1_id, 4, 1, file);
    fwrite(&subchunk1_size, sizeof(subchunk1_size), 1, file);
    fwrite(&audio_format, sizeof(audio_format), 1, file);
    fwrite(&num_channels, sizeof(num_channels), 1, file);
    fwrite(&sample_rate, sizeof(sample_rate), 1, file);
    fwrite(&byte_rate, sizeof(byte_rate), 1, file);
    fwrite(&block_align, sizeof(block_align), 1, file);
    fwrite(&bits_per_sample, sizeof(bits_per_sample), 1, file);
    fwrite(subchunk2_id, 4, 1, file);
    fwrite(&subchunk2_size, sizeof(subchunk2_size), 1, file);
}

int main(int argc, char *argv[]) {
    const char *output_wav_path = NULL;
    int32_t device_index = -1;

    int c;
    while ((c = getopt_long(argc, argv, "so:d:", long_options, NULL)) != -1) {
        switch (c) {
            case 's':
                show_audio_devices();
                return 0;
            case 'o':
                output_wav_path = optarg;
                break;
            case 'd':
                device_index = (int32_t) strtol(optarg, NULL, 10);
                break;
            default:
                exit(1);
        }
    }

    if (!output_wav_path) {
        print_usage(argv[0]);
        exit(1);
    }

    signal(SIGINT, interrupt_handler);
    fprintf(stdout, "pv_recorder version: %s\n", pv_recorder_version());

    fprintf(stdout, "Initializing pv_recorder...\n");
    const int32_t frame_length = 512;
    pv_recorder_t *recorder = NULL;
    pv_recorder_status_t status = pv_recorder_init(
            frame_length,
            device_index,
            10,
            &recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to initialize device with %s.\n", pv_recorder_status_to_string(status));
        exit(1);
    }

    pv_recorder_set_debug_logging(recorder, true);

    const char *selected_device = pv_recorder_get_selected_device(recorder);
    fprintf(stdout, "Selected device: %s.\n", selected_device);

    fprintf(stdout, "Start recording...\n");
    status = pv_recorder_start(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to start device with %s.\n", pv_recorder_status_to_string(status));
        exit(1);
    }

    int16_t *pcm = malloc(frame_length * sizeof(int16_t));
    if (!pcm) {
        fprintf(stderr, "Failed to allocate pcm memory.\n");
        exit(1);
    }

    FILE *file = fopen(output_wav_path, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file.\n");
        exit(1);
    }

    int32_t num_sample_recorded = 0;
    write_wav_header(file, num_sample_recorded);

    while (!is_interrupted) {
        status = pv_recorder_read(recorder, pcm);
        if (status != PV_RECORDER_STATUS_SUCCESS) {
            fprintf(stderr, "Failed to read with %s.\n", pv_recorder_status_to_string(status));
            exit(1);
        }

        const size_t length = fwrite(pcm, sizeof(int16_t), frame_length, file);
        if (length != frame_length) {
            fprintf(stderr, "Failed to write bytes to file.\n");
            exit(1);
        }

        num_sample_recorded += frame_length;
    }

    rewind(file);
    write_wav_header(file, num_sample_recorded);

    fclose(file);

    fprintf(stdout, "Stop recording...\n");
    status = pv_recorder_stop(recorder);
    if (status != PV_RECORDER_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to stop device with %s.\n", pv_recorder_status_to_string(status));
        exit(1);
    }

    fprintf(stdout, "Deleting pv_recorder...\n");
    pv_recorder_delete(recorder);
    free(pcm);

    return 0;
}