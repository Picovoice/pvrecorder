/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/
use hound;
use pvrecorder::RecorderBuilder;

const FRAME_LENGTH: usize = 512;
const SAMPLE_RATE: usize = 16000;
const FRAMES_TO_REC: usize = 5 * (SAMPLE_RATE / FRAME_LENGTH);

fn main() {
    println!("Printing audio devices...");
    let audio_devices = RecorderBuilder::new().get_audio_devices();
    match audio_devices {
        Ok(audio_devices) => {
            for (idx, device) in audio_devices.iter().enumerate() {
                println!("{}: {:?}", idx, device);
            }
        }
        Err(err) => panic!("Failed to get audio devices: {}", err),
    };
    println!("");

    println!("Initializing pvrecorder...");
    let recorder = RecorderBuilder::new()
        .frame_length(FRAME_LENGTH as i32)
        .init()
        .expect("Failed to initialize pvrecorder");

    println!("Start recording...");
    recorder.start().expect("Failed to start audio recording");
    let mut audio_data = Vec::new();
    for _ in 0..FRAMES_TO_REC {
        let mut frame_buffer: [i16; FRAME_LENGTH] = [0; FRAME_LENGTH];
        recorder
            .read(&mut frame_buffer)
            .expect("Failed to read audio frame");
        audio_data.extend_from_slice(&frame_buffer);
    }

    println!("Stop recording...");
    recorder.start().expect("Failed to stop audio recording");

    println!("Dumping audio to file...");
    let spec = hound::WavSpec {
        channels: 1,
        sample_rate: SAMPLE_RATE as u32,
        bits_per_sample: 16,
        sample_format: hound::SampleFormat::Int,
    };
    let mut writer = hound::WavWriter::create("debug.wav", spec).unwrap();
    for sample in audio_data {
        writer.write_sample(sample).unwrap();
    }
}
