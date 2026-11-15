// TTN uplink decoder for heltec-lora-boatreadings (port 2, 16 bytes, big-endian).
//
// The key names and units below are what the Node-RED flow in ../nodered/flow.json
// expects: it derives the Home Assistant device class and unit from the key name
// (temperature, humidity, pressure, *volt*/battery, battery_level, distance).
function decodeUplink(input) {
  var b = input.bytes;
  if (b.length !== 16) {
    return { errors: ["Unexpected payload length: " + b.length + " (expected 16)"] };
  }
  var u16 = function (i) { return (b[i] << 8) | b[i + 1]; };
  var i16 = function (i) { var v = u16(i); return v > 0x7fff ? v - 0x10000 : v; };
  var u32 = function (i) {
    return ((b[i] << 24) | (b[i + 1] << 16) | (b[i + 2] << 8) | b[i + 3]) >>> 0;
  };

  var distance = u16(12);
  var data = {
    temperature: i16(0) / 100,        // °C
    humidity: u16(2),                 // %
    pressure: u32(4) / 100,           // hPa (firmware sends Pa)
    battery: u16(8) / 1000,           // V, supply of the Heltec board
    battery_level: u16(10),           // %, Heltec battery level
    distance: distance,               // cm to the water, 0 = no echo
    external_voltage: u16(14) / 1000  // V, boat battery via the voltage divider
  };
  var warnings = [];
  if (distance === 0) warnings.push("No echo from distance sensor");
  return { data: data, warnings: warnings };
}
