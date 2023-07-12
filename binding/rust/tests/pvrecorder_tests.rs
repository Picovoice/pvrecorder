/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#[cfg(test)]
mod tests {
    use pv_recorder::{RecorderBuilder, RecorderError};

    #[test]
    fn test_init() -> Result<(), RecorderError> {
        let recorder = RecorderBuilder::new().device_index(0).init()?;
        assert!(recorder.sample_rate() > 0);
        assert!(recorder.selected_device().len() > 0);
        assert!(recorder.version().len() > 0);

        Ok(())
    }

    #[test]
    fn test_start_stop() -> Result<(), RecorderError> {
        let frame_length = 666;

        let recorder = RecorderBuilder::new().device_index(0).frame_length(frame_length).init()?;
        recorder.set_debug_logging(true);

        assert!(recorder.is_recording() == false);
        recorder.start()?;
        assert!(recorder.is_recording() == true);

        let frame = recorder.read()?;
        assert!(frame.len() == frame_length as usize);

        recorder.stop()?;
        assert!(recorder.is_recording() == false);

        Ok(())
    }

    #[test]
    fn test_get_audio_devices() -> Result<(), RecorderError> {
        let devices = RecorderBuilder::default().get_available_devices()?;

        for device in devices {
            assert!(device.len() >= 0)
        }

        Ok(())
    }
}
