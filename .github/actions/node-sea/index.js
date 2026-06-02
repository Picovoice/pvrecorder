const path = require('path');
const addonPath = path.join(__dirname, '../../../lib/node/windows/arm64/pv_recorder.node');    
const addon = require(addonPath);
const devices = addon.get_available_devices();
console.log("Success! Found", devices.length, "audio device(s):");
devices.forEach((d, i) => console.log(`  [${i}] ${d}`));