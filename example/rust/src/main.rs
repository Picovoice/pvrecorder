/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/
use clap::{App, Arg};
use hound;
use pv_recorder::{Recorder, RecorderBuilder};

const FRAME_LENGTH: usize = 512;
const SAMPLE_RATE: usize = 16000;

fn show_audio_devices() {
    println!("Printing audio devices...");
    let audio_devices = Recorder::get_audio_devices();
    match audio_devices {
        Ok(audio_devices) => {
            for (idx, device) in audio_devices.iter().enumerate() {
                println!("{}: {:?}", idx, device);
            }
        }
        Err(err) => panic!("Failed to get audio devices: {}", err),
    };
    println!("");
}

fn main() {
    let matches = App::new("PvRecorder Demo")
        .arg(
            Arg::with_name("audio_device_index")
                .long("audio_device_index")
                .value_name("INDEX")
                .help("Index of input audio device.")
                .takes_value(true)
                .default_value("-1"),
        )
        .arg(
            Arg::with_name("recording_length")
                .long("recording_length")
                .value_name("SECONDS")
                .help("Length of test audio recording in seconds.")
                .takes_value(true)
                .default_value("5"),
        )
        .arg(
            Arg::with_name("output_path")
                .long("output_path")
                .value_name("PATH")
                .help("Path to write recorded audio wav file to.")
                .takes_value(true)
                .default_value("example.wav"),
        )
        .arg(Arg::with_name("show_audio_devices").long("show_audio_devices"))
        .get_matches();

    if matches.is_present("show_audio_devices") {
        return show_audio_devices();
    }

    let audio_device_index = matches
        .value_of("audio_device_index")
        .unwrap()
        .parse()
        .unwrap();

    let recording_length: usize = matches
        .value_of("recording_length")
        .unwrap()
        .parse()
        .unwrap();

    let output_path = matches.value_of("output_path").unwrap();

    let frames_to_rec: usize = recording_length * (SAMPLE_RATE / FRAME_LENGTH);

    println!("Initializing pvrecorder...");
    let recorder = RecorderBuilder::new()
        .device_index(audio_device_index)
        .frame_length(FRAME_LENGTH as i32)
        .buffer_size_msec((recording_length * 1000) as i32)
        .init()
        .expect("Failed to initialize pvrecorder");

    println!("Start recording...");
    recorder.start().expect("Failed to start audio recording");
    let mut audio_data = Vec::new();
    for _ in 0..frames_to_rec {
        let mut frame_buffer: [i16; FRAME_LENGTH] = [0; FRAME_LENGTH];
        recorder
            .read(&mut frame_buffer)
            .expect("Failed to read audio frame");
        audio_data.extend_from_slice(&frame_buffer);
    }

    println!("Stop recording...");
    recorder.stop().expect("Failed to stop audio recording");

    println!("Dumping audio to file...");
    let spec = hound::WavSpec {
        channels: 1,
        sample_rate: SAMPLE_RATE as u32,
        bits_per_sample: 16,
        sample_format: hound::SampleFormat::Int,
    };
    let mut writer = hound::WavWriter::create(output_path, spec).unwrap();
    for sample in audio_data {
        writer.write_sample(sample).unwrap();
    }
}
