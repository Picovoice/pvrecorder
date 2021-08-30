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

class PvRecorderStatusOutOfMemoryError extends Error{}
class PvRecorderStatusInvalidArgumentError extends Error{}
class PvRecorderStatusInvalidStateError extends Error{}
class PvRecorderStatusBackendError extends Error{}
class PvRecorderStatusDeviceAlreadyInitializedError extends Error{}
class PvRecorderStatusDeviceNotInitializedError extends Error{}
class PvRecorderStatusIOError extends Error{}
class PvRecorderStatusRuntimeError extends Error{}

const PvRecorderStatus = require("./pv_recorder_status_t");

function PvRecorderStatusToException(status, errorMessage) {
    switch (status) {
        case PvRecorderStatus.OUT_OF_MEMORY:
            throw new PvRecorderStatusOutOfMemoryError(errorMessage);
        case PvRecorderStatus.INVALID_ARGUMENT:
            throw new PvRecorderStatusInvalidArgumentError(errorMessage);
        case PvRecorderStatus.INVALID_STATE:
            throw new PvRecorderStatusInvalidStateError(errorMessage);
        case PvRecorderStatus.BACKEND_ERROR:
            throw new PvRecorderStatusBackendError(errorMessage);
        case PvRecorderStatus.DEVICE_ALREADY_INITIALIZED:
            throw new PvRecorderStatusDeviceAlreadyInitializedError(errorMessage);
        case PvRecorderStatus.DEVICE_NOT_INITIALIZED:
            throw new PvRecorderStatusDeviceNotInitializedError(errorMessage);
        case PvRecorderStatus.IO_ERROR:
            throw new PvRecorderStatusIOError(errorMessage);
        case PvRecorderStatus.RUNTIME_ERROR:
            throw new PvRecorderStatusRuntimeError(errorMessage);
        default:
            console.warn(`Unknown error code: ${status}`);
            throw new Error(errorMessage);
    }
}

module.exports = PvRecorderStatusToException;