# PV_Recorder

## Requirements

- CMake 3.4+.
- C99 compatible compiler.
- **Windows**: MinGW.

## Compatibility

- Windows (amd64)
- macOS (x86_64)
- Linux (x86_64)
- Raspberry Pi:
    - Zero
    - 2
    - 3 (32 and 64 bit)
    - 4 (32 and 64 bit)
- NVIDIA Jetson Nano
- BeagleBone

## Compiling

The variable `{OUTPUT_DIR}` will be used to select the directory to copy the shared object
after a successful compilation. `{OUTPUT_DIR}` should be a directory **relative** to the root directory.

Run the following commands to build and test (`{OUTPUT_DIR}` can be empty if you wish not to copy):

```console
mkdir build && cd build
cmake .. -DOUTPUT_DIR={OUTPUT_DIR}
cmake --build .
```

If you want to build and run the demo file replace the last line in the previous command and run:

```console
cmake --build . -t demo
./demo
```