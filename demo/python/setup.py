import os
import shutil

import setuptools

os.system('git clean -dfx')

package_folder = os.path.join(os.path.dirname(__file__), 'pvrecorderdemo')
os.mkdir(package_folder)

shutil.copy(os.path.join(os.path.dirname(__file__), '../../LICENSE'), package_folder)

shutil.copy(
    os.path.join(os.path.dirname(__file__), 'pv_recorder_demo.py'),
    os.path.join(package_folder, 'pv_recorder_demo.py'))

with open(os.path.join(os.path.dirname(__file__), 'MANIFEST.in'), 'w') as f:
    f.write('include pvrecorderdemo/LICENSE\n')
    f.write('include pvrecorderdemo/pv_recorder_demo.py\n')

with open(os.path.join(os.path.dirname(__file__), 'README.md'), 'r') as f:
    long_description = f.read()

setuptools.setup(
    name="pvrecorderdemo",
    version="1.2.2",
    author="Picovoice",
    author_email="hello@picovoice.ai",
    description="Recorder library for Picovoice.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/Picovoice/pvrecorder",
    packages=["pvrecorderdemo"],
    install_requires=["pvrecorder==1.2.1"],
    include_package_data=True,
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Topic :: Multimedia :: Sound/Audio :: Speech"
    ],
    entry_points=dict(
        console_scripts=[
            'pv_recorder_demo=pvrecorderdemo.pv_recorder_demo:main',
        ],
    ),
    python_requires='>=3.5',
    keywords="Audio Recorder",
)
