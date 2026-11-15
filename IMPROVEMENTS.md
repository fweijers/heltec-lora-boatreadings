# Improvement proposals

Prioritized ideas for this project. Items marked ✅ are already done in this repository.

## Quick wins

| # | Proposal | Why | Files |
|---|----------|-----|-------|
| 1 | ✅ Keep TTN keys out of git: `ttnparams.example.h` + `.gitignore` | `ttnparams.h` was tracked, so a real AppKey could be committed by accident | `include/`, `.gitignore` |
| 2 | ✅ TTN payload decoder | Decode the 16-byte payload in TTN itself, so Node-RED receives readable JSON and no byte-math lives in the flow | `ttn/payload-formatter.js` |
| 3 | ✅ README with photos, wiring, payload table | Makes the project reproducible | `README.md` |
| 4 | Export the Node-RED flow and a Home Assistant dashboard YAML | Currently the second half of the chain can't be rebuilt from the repo | `nodered/flow.json`, `homeassistant/` |
| 5 | Move `rain.PNG` from the repo root into `pics/` and compress the 3 MB photos (about 1600 px wide is plenty) | Faster README loading, smaller clone | `pics/` |
| 6 | Add a `LICENSE` (for example MIT) | Without one, others may not reuse the project | `LICENSE` |

## Firmware

| # | Proposal | Why |
|---|----------|-----|
| 7 | Take the **median of 5 distance readings** instead of one | The graph shows the distance flickering 43–46 cm in steady conditions; a median removes single-echo outliers |
| 8 | Send `0xFFFF` instead of `0` when there is no echo | `0` currently looks like a real reading; a distinct error value lets Home Assistant ignore or alert on it (the decoder already warns on 0) |
| 9 | Average 8–16 ADC samples for the battery voltage | Reduces noise on the 12-bit ADC; the divider factor is currently set to 6.00 while 56 kΩ / 11 kΩ gives 6.09, so keep the calibrated value but document it |
| 10 | Shorten the interval when the water level or voltage changes (for example every 30 s during a pump cycle, 3 min otherwise) | A pump run can be shorter than the 3-minute interval and might be missed; remember LoRaWAN duty-cycle and TTN fair-use limits |
| 11 | Put calibration constants (`ADC_VREF`, `EXTERNAL_DIVISION_FACTOR`, sensor-to-bottom distance) in one config header | Easier to calibrate without touching code |
| 12 | Convert distance to *water height* in the decoder (`bottom_distance_cm - distance_cm`) | Rising water then shows as a rising number |
| 13 | Add a PlatformIO `platformio.ini` | Reproducible build without a specific Arduino IDE setup (needs testing on the board) |

## Alerting (Home Assistant)

| # | Proposal | Why |
|---|----------|-----|
| 14 | Notification when the boat battery drops below a threshold (for example 12.0 V) for more than 30 minutes | This is the failure that sank the boat |
| 15 | Notification when the water level stays above nominal for N readings | Pump failed or is not keeping up |
| 16 | Notification when no uplink arrives for 15 minutes | A silent sensor is as bad as a bad sensor |
| 17 | Count pump runs per day (voltage dips) and show them on the dashboard | Trend of rain and pump wear |

## Documentation

| # | Proposal | Why |
|---|----------|-----|
| 18 | Add a parts list with links and approximate prices | Helps anyone rebuilding it |
| 19 | Add a photo and notes on mounting the tube and sensor | The tube position determines your measurement range |
| 20 | Add a short "Troubleshooting" section (no join, distance always 0, voltage off) | Saves time later |
