//
// Copyright 2021-2022 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
// file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
"use strict";

enum PvRecorderStatus {
  SUCCESS = 0,
  OUT_OF_MEMORY,
  INVALID_ARGUMENT,
  INVALID_STATE,
  BACKEND_ERROR,
  DEVICE_ALREADY_INITIALIZED,
  DEVICE_NOT_INITIALIZED,
  IO_ERROR,
  RUNTIME_ERROR,
}

export default PvRecorderStatus;
