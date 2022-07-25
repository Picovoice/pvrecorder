#
# Copyright 2021-2022 Picovoice Inc.
#
# You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
# file accompanying this source.
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
# an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.
#

import os
import shutil

import setuptools

os.system('git clean -dfx')

package_folder = os.path.join(os.path.dirname(__file__), 'pvrecorder')
os.mkdir(package_folder)

shutil.copy(os.path.join(os.path.dirname(__file__), '../../LICENSE'), package_folder)

shutil.copy(os.path.join(os.path.dirname(__file__), '__init__.py'), os.path.join(package_folder, '__init__.py'))
shutil.copy(os.path.join(os.path.dirname(__file__), 'pvrecorder.py'), os.path.join(package_folder, 'pvrecorder.py'))

shutil.copytree(
    os.path.join(os.path.dirname(__file__), '../../scripts'),
    os.path.join(package_folder, 'scripts'))

platforms = ('beaglebone', 'jetson', 'linux', 'mac', 'raspberry-pi', 'windows')

os.mkdir(os.path.join(package_folder, 'lib'))
for platform in platforms:
    shutil.copytree(
        os.path.join(os.path.dirname(__file__), '../../lib', platform),
        os.path.join(package_folder, 'lib', platform))

MANIFEST_IN = """
include pvrecorder/LICENSE
include pvrecorder/__init__.py
include pvrecorder/pv_recorder.py
include pvrecorder/lib/beaglebone/libpv_recorder.so
recursive-include pvrecorder/lib/jetson *
include pvrecorder/lib/linux/x86_64/libpv_recorder.so
include pvrecorder/lib/mac/x86_64/libpv_recorder.dylib
include pvrecorder/lib/mac/arm64/libpv_recorder.dylib
recursive-include pvrecorder/lib/raspberry-pi *
include pvrecorder/lib/windows/amd64/libpv_recorder.dll
recursive-include pvrecorder/scripts *
"""

with open(os.path.join(os.path.dirname(__file__), 'MANIFEST.in'), 'w') as f:
    f.write(MANIFEST_IN.strip('\n '))

with open(os.path.join(os.path.dirname(__file__), 'README.md'), 'r') as f:
    long_description = f.read()

setuptools.setup(
    name="pvrecorder",
    version="1.1.1",
    author="Picovoice",
    author_email="hello@picovoice.ai",
    description="Recorder library for Picovoice.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/Picovoice/pvrecorder",
    packages=["pvrecorder"],
    install_requires=[],
    include_package_data=True,
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Topic :: Multimedia :: Sound/Audio :: Speech"
    ],
    python_requires='>=3.5',
    keywords="audio recorder",
)
