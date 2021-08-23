from pvrecorder import PVRecorder


def main():

    devices = PVRecorder.get_audio_devices()
    for i in range(len(devices)):
        print("index: %d, device name: %s" % (i, devices[i]))

    def callback(pcm):
        # do something
        print(len(pcm))

    recorder = PVRecorder(device_index=-1, frame_length=512, callback=callback)
    recorder.start()

    input("Press any key to continue...\n")

    recorder.delete()



if __name__ == "__main__":
    main()