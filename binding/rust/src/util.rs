/*
    Copyright 2021-2025 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

use std::path::PathBuf;

const DEFAULT_RELATIVE_LIBRARY_DIR: &str = "lib/";

#[cfg(all(target_os = "linux", any(target_arch = "arm", target_arch = "aarch64")))]
fn find_machine_type() -> String {
    use std::process::Command;

    let cpu_info = Command::new("cat")
        .arg("/proc/cpuinfo")
        .output()
        .expect("Failed to retrieve cpu info");
    let cpu_part_list = std::str::from_utf8(&cpu_info.stdout)
        .unwrap()
        .split("\n")
        .filter(|x| x.contains("CPU part"))
        .collect::<Vec<_>>();

    if cpu_part_list.len() == 0 {
        panic!("Unsupported CPU");
    }

    let cpu_part = cpu_part_list[0]
        .split(" ")
        .collect::<Vec<_>>()
        .pop()
        .unwrap()
        .to_lowercase();

    let machine = match cpu_part.as_str() {
        "0xb76" => "arm11",
        "0xd03" => "cortex-a53",
        "0xd08" => "cortex-a72",
        "0xd0b" => "cortex-a76",
        _ => "unsupported",
    };

    String::from(machine)
}

#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
fn base_library_path() -> PathBuf {
    PathBuf::from("mac/x86_64/libpv_recorder.dylib")
}

#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
fn base_library_path() -> PathBuf {
    PathBuf::from("mac/arm64/libpv_recorder.dylib")
}

#[cfg(all(target_os = "windows", target_arch = "x86_64"))]
fn base_library_path() -> PathBuf {
    PathBuf::from("windows/amd64/libpv_recorder.dll")
}

#[cfg(all(target_os = "windows", target_arch = "aarch64"))]
fn base_library_path() -> PathBuf {
    PathBuf::from("windows/arm64/libpv_recorder.dll")
}

#[cfg(all(target_os = "linux", target_arch = "x86_64"))]
fn base_library_path() -> PathBuf {
    PathBuf::from("linux/x86_64/libpv_recorder.so")
}

#[cfg(all(target_os = "linux", any(target_arch = "arm", target_arch = "aarch64")))]
fn base_library_path() -> PathBuf {
    const RPI_MACHINES: [&str; 4] = ["arm11", "cortex-a53", "cortex-a72", "cortex-a76"];

    let machine = find_machine_type();
    match machine.as_str() {
        machine if RPI_MACHINES.contains(&machine) => {
            if cfg!(target_arch = "aarch64") {
                PathBuf::from(format!(
                    "raspberry-pi/{}-aarch64/libpv_recorder.so",
                    &machine
                ))
            } else {
                PathBuf::from(format!("raspberry-pi/{}/libpv_recorder.so", &machine))
            }
        }
        _ => {
            eprintln!("WARNING: Please be advised that this device is not officially supported by Picovoice.\nFalling back to the armv6-based (Raspberry Pi Zero) library. This is not tested nor optimal.\nFor the model, use Raspberry Pi's models");
            PathBuf::from("raspberry-pi/arm11/libpv_recorder.so")
        }
    }
}

pub fn pv_library_path() -> PathBuf {
    PathBuf::from(env!("OUT_DIR"))
        .join(DEFAULT_RELATIVE_LIBRARY_DIR)
        .join(base_library_path())
}
