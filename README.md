# heltec-lora-boatreadings

Remote bilge monitoring for my boat: water level, battery voltage and weather, sent over LoRaWAN (The Things Network) to Home Assistant.

![The monitoring box in the bow of the boat, next to the solar panel and the charge controller](pics/CompuInBoat.JPG)

## Why this project?

I found out the hard way that a bilge pump setup can falter: in my case it resulted in a sunken boat (a *sloep*, in Dutch).

My bilge pump runs on a 12 V battery, which is charged by a small solar panel. A solar charge controller charges the battery and protects it from being drained below a certain level, for example after a lot of rain and little sun. As long as there is enough sunshine and no connection breaks because of storm, rain or bad luck, the pump automatically removes rain water from the boat.

Because connections can break and the battery can run low, I wanted to measure **the battery voltage and the bilge water level remotely**.

## What does it show?

### No rain
The battery voltage drops by about a volt during the night and is charged back to its nominal level during the day. The bilge water level stays the same.

### Rain
The bilge water level slowly rises until the battery voltage shows a sudden dip: the bilge pump has started. After that the water level is back to nominal.

The graph below shows exactly that. Each dip in the battery voltage (top) lines up with a change in the measured distance to the water (bottom): the pump kicks in and the level drops again.

![Battery voltage (top) and measured distance to the water (bottom); the drawn lines connect each voltage dip to the matching change in water level](pics/correlation-voltage-vs-distance.jpeg)

The rain radar of the same morning shows the showers passing over the boat:

![Rain radar showing showers over the area at the time of the voltage dips](pics/rain.jpg)

## How is it built?

```
JSN-SR04T ─┐
BME280 ────┼─► Heltec AB01 V2 ──LoRaWAN──► The Things Network ──MQTT──► Node-RED ──► Home Assistant
12 V divider ┘   (CubeCell)
```

### Hardware

| Part | Purpose |
|------|---------|
| Heltec CubeCell HTCC-AB01 V2 | Microcontroller with LoRa radio |
| JSN-SR04T | Waterproof ultrasonic distance sensor, measures the distance to the water in the bilge |
| BME280 | Bonus: air temperature, humidity and pressure |
| Voltage divider (56 kΩ / 11 kΩ) | Brings the ~13.8 V battery voltage below 3 V for the ADC pin |
| 10 kΩ / transistor + 18 kΩ / 33 kΩ divider | Protects the 3.3 V Heltec from the 5 V JSN-SR04T (supply switching and echo level) |
| 12 V battery, small solar panel, solar charge controller | Power for the bilge pump and for this monitor |

### Wiring

![Wiring diagram: Heltec pins, BME280, JSN-SR04T, level protection and battery voltage divider](pics/BOOTschema.jpg)

| Signal | Heltec pin |
|--------|-----------|
| JSN-SR04T trigger | GPIO5 (J2-9) |
| JSN-SR04T echo | GPIO0 (J2-8), via the 18 kΩ / 33 kΩ divider |
| Battery voltage | ADC (J3-11), via the 56 kΩ / 11 kΩ divider |
| BME280 | I²C: SCL (J2-6), SDA (J2-7), powered from Vext (J2-4) |

> The firmware uses a measured division factor (`EXTERNAL_DIVISION_FACTOR`) and a measured ADC reference voltage (`ADC_VREF`). Calibrate both against a multimeter for your own board.

### Photos

The sensor looks down a tube that sticks into the bilge:

![The tube with the distance sensor, sticking into the bilge](pics/BilgeMeasurementTube.JPG)

The electronics sit in a watertight box:

![The Heltec board, BME280, JSN-SR04T and the level-protection board in the box](pics/HeltecInBox.JPG)

The solar charge controller feeds the battery and shields it against deep discharge:

![Solar charge controller on top of the electronics box](pics/SolarController.JPG)

## Firmware

The sketch is in [`src/HELTEC_HTCC_AB_V2_BME280_JSN_SR04T_september2025.ino`](src/HELTEC_HTCC_AB_V2_BME280_JSN_SR04T_september2025.ino).

1. Install the Arduino IDE with the Heltec **CubeCell** board package and the `Seeed_BME280` library.
2. Copy `include/ttnparams.example.h` to `include/ttnparams.h` and fill in your own DevEUI, AppEUI and AppKey from the TTN console. This file is git-ignored, so your keys never end up in the repository.
3. Select the CubeCell HTCC-AB01 V2 board, choose the LoRaWAN region (EU868 in the Netherlands), OTAA and class A in the Tools menu, then upload.

The node sends one uplink every ~3 minutes (`appTxDutyCycle`), with the sensors only powered while measuring (Vext).

### Payload (port 2, 16 bytes, big-endian)

| Bytes | Field | Unit / scaling |
|-------|-------|----------------|
| 0–1 | Temperature | signed, °C × 100 |
| 2–3 | Humidity | % |
| 4–7 | Pressure | Pa |
| 8–9 | Heltec supply voltage | mV |
| 10–11 | Heltec battery level | % |
| 12–13 | Distance to water | cm (`0` = no echo) |
| 14–15 | External (boat) battery voltage | mV |

A ready-made TTN uplink decoder is in [`ttn/payload-formatter.js`](ttn/payload-formatter.js): paste it in the TTN console under *Payload formatters → Uplink → Custom JavaScript*.

## The Things Network and Home Assistant

Data is shown in Home Assistant. Node-RED retrieves the data from TTN and stores it in Home Assistant entities, which are displayed in a Lovelace dashboard:

```
TTN → MQTT → Node-RED → MQTT auto-discovery in Home Assistant
```

See [`IMPROVEMENTS.md`](IMPROVEMENTS.md) for ideas on where to take this project next.
