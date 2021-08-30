//
// Copyright 2021 Picovoice Inc.
//
// You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
// file accompanying this source.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
"use strict";

const PvRecorderStatus = {
    SUCCESS: 0,
    OUT_OF_MEMORY: 1,
    INVALID_ARGUMENT: 2,
    INVALID_STATE: 3,
    BACKEND_ERROR: 4,
    DEVICE_ALREADY_INITIALIZED: 5,
    DEVICE_NOT_INITIALIZED: 6,
    IO_ERROR: 7,
    RUNTIME_ERROR: 8,
};

module.exports = PvRecorderStatus;