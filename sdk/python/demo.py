from pvrecorder import PVRecorder


def main():
    devices = PVRecorder.get_audio_devices()
    for i in range(len(devices)):
        print("index: %d, device name: %s" % (i, devices[i]))

    recorder = PVRecorder(device_index=-1)
    recorder.start()

    try:
        while True:
            pcm = recorder.read(512)
            # do something with pcm
    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        recorder.delete()


if __name__ == "__main__":
    main()
