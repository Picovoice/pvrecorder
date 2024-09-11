import { PvRecorder } from "../src";

describe("Test PvRecorder", () => {
  test("invalid device index", () => {
    const f = () => {
      new PvRecorder(512, -2);
    };

    expect(f).toThrow(Error);
  });

  test("invalid frame length", () => {
    const f = () => {
      new PvRecorder(0, 0);
    };

    expect(f).toThrow(Error);
  });

  test("invalid buffered frames count", () => {
    const f = () => {
      new PvRecorder(512, 0, 0);
    };

    expect(f).toThrow(Error);
  });

  test("start stop", async () => {
    const recorder = new PvRecorder(512, 0);
    recorder.start();

    for (let i = 0; i < 5; i++) {
      const frames = recorder.readSync();
      expect(frames.length).toEqual(recorder.frameLength);
    }

    for (let i = 0; i < 5; i++) {
      const frames = await recorder.read();
      expect(frames.length).toEqual(recorder.frameLength);
    }

    recorder.release();
  });

  test("set debug logging", () => {
    const recorder = new PvRecorder(512, 0);
    recorder.setDebugLogging(true);
    recorder.setDebugLogging(false);
    recorder.release();
  });

  test("is recording", () => {
    const recorder = new PvRecorder(512, 0);

    recorder.start();
    expect(recorder.isRecording).toBeTruthy();

    recorder.stop();
    expect(recorder.isRecording).toBeFalsy();

    recorder.release();
  });

  test("get selected device", () => {
    const recorder = new PvRecorder(512, 0);
    const device = recorder.getSelectedDevice();

    expect(device).toBeDefined();
    expect(typeof device).toBe("string");

    recorder.release();
  });

  test("get available devices", () => {
    const devices = PvRecorder.getAvailableDevices();

    for (const device of devices) {
      expect(device).toBeDefined();
      expect(typeof device).toBe("string");
    }
  });

  test("version", () => {
    const recorder = new PvRecorder(512, 0);

    expect(recorder.version).toBeDefined();
    expect(typeof recorder.version).toBe("string");
    expect(recorder.version.length).toBeGreaterThan(0);

    recorder.release();
  });

  test("sample rate", () => {
    const recorder = new PvRecorder(512, 0);

    expect(recorder.sampleRate).toBeDefined();
    expect(typeof recorder.sampleRate).toBe("number");
    expect(recorder.sampleRate).toBeGreaterThan(0);

    recorder.release();
  });
});
