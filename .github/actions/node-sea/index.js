const path = require('path');
const { createRequire } = require('module');

const addonPath = path.join(__dirname, '../../../lib/node/windows/arm64/pv_recorder.node');    
const requireFromDisk = createRequire(process.execPath);

try {
    const addon = requireFromDisk(addonPath);
    const devices = addon.get_available_devices();
    console.log("Found", devices.length, "audio device(s):");
    devices.forEach((d, i) => console.log(`  [${i}] ${d}`));
} catch (e) {
    console.error("Failed to load addon:", e);
    process.exit(1);
}