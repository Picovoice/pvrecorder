/*
    Copyright 2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

using System;

namespace Pv
{
    public class PvRecorderException : Exception
    {
        public PvRecorderException() { }

        public PvRecorderException(string message) : base(message) { }

    }

    public class PvRecorderMemoryException : PvRecorderException
    {
        public PvRecorderMemoryException() { }

        public PvRecorderMemoryException(string message) : base(message) { }
    }

    public class PvRecorderInvalidArgumentException : PvRecorderException
    {
        public PvRecorderInvalidArgumentException() { }

        public PvRecorderInvalidArgumentException(string message) : base(message) { }
    }

    public class PvRecorderInvalidStateException : PvRecorderException
    {
        public PvRecorderInvalidStateException() { }

        public PvRecorderInvalidStateException(string message) : base(message) { }
    }

    public class PvRecorderBackendException : PvRecorderException
    {
        public PvRecorderBackendException() { }

        public PvRecorderBackendException(string message) : base(message) { }
    }

    public class PvRecorderDeviceAlreadyInitializedException : PvRecorderException
    {
        public PvRecorderDeviceAlreadyInitializedException() { }

        public PvRecorderDeviceAlreadyInitializedException(string message) : base(message) { }
    }

    public class PvRecorderDeviceNotInitializedException : PvRecorderException
    {
        public PvRecorderDeviceNotInitializedException() { }

        public PvRecorderDeviceNotInitializedException(string message) : base(message) { }
    }


    public class PvRecorderIOException : PvRecorderException
    {
        public PvRecorderIOException() { }

        public PvRecorderIOException(string message) : base(message) { }
    }

    public class PvRecorderRuntimeException : PvRecorderException
    {
        public PvRecorderRuntimeException() { }

        public PvRecorderRuntimeException(string message) : base(message) { }
    }
}