/*
    Copyright 2021-2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/
use clap::{value_parser, Arg, ArgAction, Command};
use pv_recorder::PvRecorderBuilder;
use std::sync::atomic::{AtomicBool, Ordering};

const SAMPLE_RATE: usize = 16000;

static LISTENING: AtomicBool = AtomicBool::new(false);

fn show_audio_devices() {
    println!("Printing audio devices...");

    let audio_devices = PvRecorderBuilder::default().get_available_devices();
    match audio_devices {
        Ok(audio_devices) => {
            for (idx, device) in audio_devices.iter().enumerate() {
                println!("{}: {:?}", idx, device);
            }
        }
        Err(err) => panic!("Failed to get audio devices: {}", err),
    };
    println!();
}

fn main() {
    let matches = Command::new("PvRecorder Demo")
        .arg(
            Arg::new("audio_device_index")
                .long("audio_device_index")
                .value_name("INDEX")
                .help("Index of input audio device.")
                .value_parser(value_parser!(i32))
                .default_value("-1"),
        )
        .arg(
            Arg::new("output_wav_path")
                .long("output_wav_path")
                .value_name("PATH")
                .help("Path to write recorded audio wav file to.")
                .default_value("example.wav"),
        )
        .arg(
            Arg::new("show_audio_devices")
                .long("show_audio_devices")
                .action(ArgAction::SetTrue),
        )
        .get_matches();

    if matches.get_flag("show_audio_devices") {
        return show_audio_devices();
    }

    let audio_device_index = *matches.get_one::<i32>("audio_device_index").unwrap();

    let output_wav_path = matches.get_one::<String>("output_wav_path").unwrap();

    println!("Initializing pvrecorder...");
    let recorder = PvRecorderBuilder::new(512)
        .device_index(audio_device_index)
        .init()
        .expect("Failed to initialize pvrecorder");

    ctrlc::set_handler(|| {
        LISTENING.store(false, Ordering::SeqCst);
    })
    .expect("Unable to setup signal handler");

    println!("Start recording...");
    recorder.start().expect("Failed to start audio recording");
    LISTENING.store(true, Ordering::SeqCst);

    let mut audio_data = Vec::new();
    while LISTENING.load(Ordering::SeqCst) {
        let frame = recorder.read().expect("Failed to read audio frame");
        audio_data.extend_from_slice(&frame);
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
    let mut writer = hound::WavWriter::create(output_wav_path, spec).unwrap();
    for sample in audio_data {
        writer.write_sample(sample).unwrap();
    }
}
