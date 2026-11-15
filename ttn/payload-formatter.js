// TTN uplink decoder for heltec-lora-boatreadings (port 2, 16 bytes, big-endian).
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
    temperature_c: i16(0) / 100,
    humidity_pct: u16(2),
    pressure_hpa: u32(4) / 100,
    heltec_voltage_mv: u16(8),
    heltec_battery_pct: u16(10),
    distance_cm: distance,
    external_voltage_v: u16(14) / 1000
  };
  var warnings = [];
  if (distance === 0) warnings.push("No echo from distance sensor");
  return { data: data, warnings: warnings };
}
