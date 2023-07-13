/*
    Copyright 2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#[cfg(test)]
mod tests {
    use pv_recorder::{PvRecorderBuilder, PvRecorderError};

    #[test]
    fn test_init() -> Result<(), PvRecorderError> {
        let recorder = PvRecorderBuilder::new(512).device_index(0).init()?;
        assert!(recorder.sample_rate() > 0);
        assert!(recorder.selected_device().len() > 0);
        assert!(recorder.version().len() > 0);

        Ok(())
    }

    #[test]
    fn test_start_stop() -> Result<(), PvRecorderError> {
        let frame_length = 666;

        let recorder = PvRecorderBuilder::new(frame_length)
            .device_index(0)
            .frame_length(frame_length)
            .init()?;
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
    fn test_get_available_devices() -> Result<(), PvRecorderError> {
        let devices = PvRecorderBuilder::default().get_available_devices()?;

        for device in devices {
            assert!(device.len() >= 0)
        }

        Ok(())
    }
}
