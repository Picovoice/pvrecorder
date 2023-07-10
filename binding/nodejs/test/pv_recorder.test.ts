import { PvRecorder } from "../src";

describe("Test PvRecorder", () => {
  test("invalid device index", () => {
    const f = () => {
      new PvRecorder(-2, 512);
    }

    expect(f).toThrow(Error);
  });

  test("invalid frame length", () => {
    const f = () => {
      new PvRecorder(-1, 0);
    }

    expect(f).toThrow(Error);
  });

  test("invalid buffered frames count", () => {
    const f = () => {
      new PvRecorder(-1, 512, 0);
    }

    expect(f).toThrow(Error);
  });

  test("start stop", async () => {
    const recorder = new PvRecorder(-1, 512);
    recorder.start();

    for (let i = 0; i < 5; i++) {
      const frames = recorder.readSync();
      expect(frames.length).toEqual(512);
    }

    for (let i = 0; i < 5; i++) {
      const frames = await recorder.read();
      expect(frames.length).toEqual(512);
    }

    recorder.release();
  });

  test("set debug logging", () => {
    const recorder = new PvRecorder(-1, 512);
    recorder.setDebugLogging(true);
    recorder.setDebugLogging(false);
    recorder.release();
  });

  test("is recording", () => {
    const recorder = new PvRecorder(-1, 512);

    recorder.start();
    expect(recorder.isRecording).toBeTruthy();

    recorder.stop();
    expect(recorder.isRecording).toBeFalsy();

    recorder.release();
  });

  test("get selected device", () => {
    const recorder = new PvRecorder(-1, 512);
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
    const recorder = new PvRecorder(-1, 512);

    expect(recorder.version).toBeDefined();
    expect(typeof recorder.version).toBe("string");
    expect(recorder.version.length).toBeGreaterThan(0);

    recorder.release();
  });

  test("sample rate", () => {
    const recorder = new PvRecorder(-1, 512);

    expect(recorder.sampleRate).toBeDefined();
    expect(typeof recorder.sampleRate).toBe("number");
    expect(recorder.sampleRate).toBeGreaterThan(0);

    recorder.release();
  });
});
