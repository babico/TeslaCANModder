# End-to-End User Scenarios & Test Plan

Board-specific test scenarios for every supported hardware configuration. Each scenario documents exact wiring, firmware environment, connectivity, and feature coverage.

---

## Board Reference

| Board | CAN Modules | CAN Buses | Connectivity | Persistence | Firmware Envs |
| ----- | ----------- | --------- | ----------- | ----------- | ------------- |
| Arduino Uno R3 | MCP2515 × 1–3 | BUS_FSD, BUS_VEHICLE, BUS_BODY (flags) | USB Serial, HC-05 BT (optional) | EEPROM | `uno`, `uno_bt` |
| ESP32 DevKit | MCP2515 × 1–3 | BUS_FSD, BUS_VEHICLE, BUS_BODY (flags) | USB Serial, WiFi AP/STA, BLE (NimBLE) | NVS (Preferences) | `esp32`, `esp32_wifi`, `esp32_ble`, `esp32_wifi_ble` |

---

## Arduino Uno Scenarios

## UNO-1: First-Time Setup — FSD Only (uno)

**User Story:** New user wires Arduino Uno + 1× MCP2515 to Tesla X179, connects via USB.

**Hardware:** Arduino Uno R3, MCP2515 (TJA1050, 8 MHz crystal), USB cable

**Wiring:**

| MCP2515 Pin | Arduino Pin | Function |
| ----------- | ----------- | -------- |
| VCC | 5V | Power |
| GND | GND | Ground |
| CS | D10 (PIN_MCP2515_1_CS) | SPI chip select |
| INT | D2 (PIN_MCP2515_1_INT) | Hardware interrupt (INT0) |
| SCK | D13 | SPI clock |
| MOSI | D11 | SPI MOSI |
| MISO | D12 | SPI MISO |
| CAN-H | X179 pin 9 or 13 | CAN bus high |
| CAN-L | X179 pin 10 or 14 | CAN bus low |

| Step | Action | Expected Result | Status |
| ---- | ------ | -------------- | ------ |
| 1 | Wire MCP2515 to Arduino per table above | Physical wiring complete | — |
| 2 | Flash `uno` firmware via PlatformIO | Upload successful at 115200 baud | — |
| 3 | Open web dashboard in Chrome | Dashboard loads, shows "Not Connected" | — |
| 4 | Click "Connect USB" | Browser Web Serial prompt appears | — |
| 5 | Select Arduino COM port | Connection established | — |
| 6 | Verify boot JSON | `{"t":"boot","hw":"ArduinoUnoR3CH340","can":"MCP2515_TJA1050_8MHz","drv":"arduino-mcp2515","busFsd":true,"busVehicle":false,"busBody":false,"bt":"HC-05","btEnabled":false,...}` | — |
| 7 | Status messages arrive every 2000ms | Uptime ticks, rate shows msg/s | — |
| 8 | MCP2515 init tries 8MHz then 16MHz | Crystal auto-detection works | — |

**Pass Criteria:** Dashboard connected, variant defaults to HW4, EEPROM defaults applied (FSD=off, nag=off, profile=0).

---

## UNO-2: Vehicle Bus Setup (uno + BUS_VEHICLE_ACTIVE=1)

**User Story:** User adds a second MCP2515 for vehicle control bus access.

**Additional Wiring (Bus 1):**

| MCP2515 #2 Pin | Arduino Pin | Function |
| --------------- | ----------- | -------- |
| CS | D9 (PIN_MCP2515_2_CS) | Bus 1 chip select |
| INT | D3 (PIN_MCP2515_2_INT) | Hardware interrupt (INT1) |
| CAN-H/CAN-L | X179 pins 9-10 | Vehicle Control bus |

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Wire second MCP2515 per table above | Both modules on SPI bus |
| 2 | Flash `uno` with `BUS_VEHICLE_ACTIVE=1` | Boot shows `busFsd:true, busVehicle:true` |
| 3 | Bus 0 receives FSD frames | 0x399, 0x3FD, 0x3F8 filtered |
| 4 | Bus 1 receives vehicle frames | 0x273, 0x2F3, 0x333, 0x334 filtered |
| 5 | Vehicle control commands work | Climate, charge, drive sent on bus 1 |
| 6 | Bus 1 init failure is graceful | Falls back to single-bus mode |

**Pass Criteria:** Both buses operational, frame routing correct per bus.

---

## UNO-3: Full Bus Setup (uno + BUS_VEHICLE_ACTIVE=1 + BUS_BODY_ACTIVE=1)

**User Story:** User adds a third MCP2515 for body control bus.

**Additional Wiring (Bus 2):**

| MCP2515 #3 Pin | Arduino Pin | Function |
| --------------- | ----------- | -------- |
| CS | D8 (PIN_MCP2515_3_CS) | Bus 2 chip select |
| INT | D6 (PIN_MCP2515_3_INT) | **Polled** (no hardware INT) |
| CAN-H/CAN-L | X179 pins 2-3 | Body Control bus |

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Wire third MCP2515 per table | Three modules sharing SPI |
| 2 | Flash `uno` with `BUS_VEHICLE_ACTIVE=1 BUS_BODY_ACTIVE=1` | Boot shows `busFsd:true, busVehicle:true, busBody:true` |
| 3 | Bus 2 is polled (no INT) | Slightly higher latency on bus 2, still functional |
| 4 | Window vent commands work | Sent on correct bus |
| 5 | Sentry commands work | Sent on correct bus |
| 6 | Trunk commands work | Sent on correct bus |

**Note:** Bus 2 on Arduino Uno uses polling instead of hardware interrupts because D6 is not an external interrupt pin. This adds minor latency but is fully functional.

---

## UNO-4: Arduino Uno + HC-05 Bluetooth (uno_bt)

**User Story:** User adds HC-05 Bluetooth module for wireless serial access alongside USB.

**HC-05 Wiring:**

| HC-05 Pin | Arduino Pin | Function |
| --------- | ----------- | -------- |
| VCC | 5V | Power |
| GND | GND | Ground |
| TXD | D4 (PIN_BT_RX) | HC-05 TX → Arduino RX (SoftwareSerial) |
| RXD | D5 (PIN_BT_TX) | Arduino TX → HC-05 RX (SoftwareSerial) |

**Note:** HC-05 communicates at 9600 baud (BT_BAUD) via SoftwareSerial. The standard USB Serial runs at 115200 baud.

| Step | Action | Expected Result | Status |
| ---- | ------ | -------------- | ------ |
| 1 | Wire HC-05 per table above | HC-05 LED blinking (pairing mode) | — |
| 2 | Flash `uno_bt` firmware | Boot message includes `btEnabled:true` | — |
| 3 | Pair phone/laptop with HC-05 | BT pairing successful, HC-05 LED solid | — |
| 4 | Open BT serial terminal (9600 baud) | Boot JSON received over BT | — |
| 5 | Send `ping` via BT serial | `{"t":"pong","v":1}` received | — |
| 6 | Send `fsd:on` via BT serial | ACK received, FSD enabled | — |
| 7 | Status messages arrive on BT | Same JSON as USB, every 2000ms | — |
| 8 | USB and BT operate simultaneously | Both receive status, both accept commands | — |
| 9 | Send `status` via USB | Full state JSON (same as BT) | — |
| 10 | Disconnect BT, USB still works | No disruption, board continues | — |
| 11 | Reconnect BT | Resume receiving status messages | — |

**HC-05 vs USB Output:** All output functions (`sendBoot`, `sendStatus`, `sendAck`, etc.) mirror to both USB Serial and BT SoftwareSerial simultaneously. Both accept the same command set.

**HC-05 Limitations:**

- 9600 baud (vs 115200 USB) — slower throughput
- SoftwareSerial on Uno — only one RX pin can listen at a time
- No encryption (HC-05 is Bluetooth Classic SPP)
- Range: ~10m line-of-sight
- Android only typically (iOS does not support SPP)

**Pass Criteria:** Commands work over both USB and BT. Boot/status/ACK messages arrive on both channels. Board stable with both active.

---

## UNO-5: FSD + Nag Configuration

**User Story:** User enables FSD and nag suppression, sets preferred speed profile.

**Applies to:** All Uno firmware variants

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Send `fsd:on` | ACK, FSD badge ON |
| 2 | Send `nag:on` | ACK, nag suppression ON |
| 3 | Send `profile:2` | Profile changes to 2 (PINNED) |
| 4 | Send `profile:auto` | Profile follows stalk position (AUTO) |
| 5 | (HW4 only) Send `isa:on` | ISA chime suppressed |
| 6 | (HW3 only) Send `offset:60` | Speed offset 60% (PINNED) |
| 7 | Send `status` | All settings reflected in status JSON |
| 8 | Power cycle board | Settings restored from EEPROM |

**EEPROM Persistence:** Magic `0xCA`, version `0x02`. Stores: variant, fsdEnabled, nagSuppress, speedProfile, profileOverride, speedOffset, offsetOverride, isaChimeSuppress.

---

## UNO-6: Vehicle Controls (Requires 0x273 Cache)

**User Story:** User sends vehicle control commands. Requires `hasCtrl=true` (0x273 frame cached from live CAN).

**Applies to:** `uno` / `uno_bt` with `BUS_VEHICLE_ACTIVE=1` (needs Vehicle bus for vehicle control)

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Wait for 0x273 frame to be cached | `hasCtrl` = true in status |
| 2 | Send `unlock` | 30× 0x273 burst, car unlocks |
| 3 | Send `lock` | 30× 0x273 burst, car locks |
| 4 | Send `frunk:open` | 50× 0x273 burst, frunk opens |
| 5 | Send `trunk:open` | 50× 0x273 burst, trunk opens |
| 6 | Send `mirror:fold` | Mirror fold burst via ctrlBus |
| 7 | Send `seat:fl:3` | Front-left seat heat level 3 |
| 8 | Send command without 0x273 cached | Error: "Waiting for 0x273 frame" |

**Frame-dependent commands:**

| Command Group | Required Cache | CAN ID |
| ------------- | ------------- | ------ |
| lock, unlock, frunk, trunk, mirror, light, wiper, seat, display, power | `hasCtrl` (0x273) | 0x273 via ctrlBus |
| climate:keep, climate:off | `hasClimate` (0x2F3) | 0x2F3 |
| charge:start, charge:stop, charge:port | `hasCharge` (0x333) | 0x333 |
| pedal:sport, pedal:chill, regen, stopmode | `hasDrive` (0x334) | 0x334 |

---

## UNO-7: Variant Switching

**User Story:** User switches between Tesla hardware variants.

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Send `variant:hw4` | Variant set, filters re-applied, handler switched |
| 2 | Features: FSD, nag, profile, ISA chime, summon | All available |
| 3 | Send `variant:hw3` | Variant switched, speed offset visible |
| 4 | Features: FSD, nag, profile, speed offset, summon | ISA chime NOT available |
| 5 | Send `variant:legacy` | Variant switched, minimal features |
| 6 | Features: FSD, nag, profile (0-2 only) | No summon, no offset, no ISA |
| 7 | Variant persists in EEPROM | Survives power cycle |

**Feature Matrix:**

| Feature | HW4 | HW3 | Legacy |
| ------- | --- | --- | ------ |
| FSD enable/disable | ✓ | ✓ | ✓ |
| Nag suppression | ✓ | ✓ | ✓ |
| Speed profile | ✓ (auto/pin) | ✓ (auto/pin) | ✓ (0-2 only) |
| Speed offset | — | ✓ | — |
| ISA chime suppress | ✓ | — | — |
| Summon | ✓ | ✓ | — |

---

## UNO-8: Summon (Remote Move)

**User Story:** User activates summon to move car forward/reverse.

**Requires:** `hasCtrl` (0x273 cached), HW4 or HW3 variant

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Verify 0x273 cached | `hasCtrl:true` in status |
| 2 | Send `summon:forward` | 30× 0x273 burst with summon bits @ 20ms |
| 3 | Car begins moving forward | Visible movement |
| 4 | Send `summon:stop` | `summonRemaining` reset to 0 |
| 5 | Send `summon:reverse` | 30× reverse burst |
| 6 | Send on Legacy variant | Error: summon not supported |

---

## UNO-9: CAN Stream & Debugging

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Send `stream:on` | Stream enabled, frames sent to serial |
| 2 | Frames show `id`, `dlc`, hex data, bus number | JSON frame messages |
| 3 | Send `can:raw:on` | All CAN IDs pass through (no filter) |
| 4 | Observe high-traffic IDs | Both bus 0 and bus 1 frames visible |
| 5 | Send `can:raw:off` | Filters re-applied per variant |
| 6 | Send `stream:off` | Frame output stops |

---

## UNO-10: Powerbank Deployment

**User Story:** User configures board, disconnects laptop, runs standalone with powerbank.

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Configure FSD=ON, Nag=ON, Profile=3 | Settings saved to EEPROM |
| 2 | Disconnect laptop USB | Board powered down |
| 3 | Connect powerbank to Arduino USB | Board reboots, loads EEPROM settings |
| 4 | Place in car, close door | Board running standalone |
| 5 | Drive — CAN frames flow | FSD/Nag/Profile mods active |
| 6 | Park, walk away | CAN bus goes silent after ~15 min |
| 7 | CAN_TIMEOUT_MS (10s) triggers | Board enters standby, LED slow-blink |
| 8 | MCP2515 in listen-only | Reduced polling, CAN_REINIT_INTERVAL=5s |
| 9 | Return, open door | CAN bus wakes, frames resume |
| 10 | Board auto-recovers | Re-applies filters, resumes operation |
| 11 | (BT) HC-05 still paired | Can reconnect BT to check status |

---

## UNO-11: Power Cycle Recovery

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Board running with FSD/Nag/Profile active | All settings active |
| 2 | Power loss (2 seconds) | Board shuts down |
| 3 | Power restored | Board reboots |
| 4 | EEPROM settings restored | FSD/Nag/Profile match pre-disconnect |
| 5 | CAN frames resume | Board applies mods immediately |
| 6 | Frame caches rebuild | hasCtrl/hasClimate/hasCharge/hasDrive repopulate from live CAN |

---

## UNO-12: Error Handling

| Case | Trigger | Expected Result |
| ---- | ------- | -------------- |
| 12.1 | Send command without CAN initialized | Error response |
| 12.2 | Send vehicle cmd without 0x273 cached | Error: "Waiting for 0x273 frame" |
| 12.3 | Send climate cmd without 0x2F3 cached | Error: "Need 0x2F3" |
| 12.4 | Send unknown command | Error: "Unknown command" |
| 12.5 | USB cable disconnect mid-session | Dashboard shows "Not Connected" |
| 12.6 | Rapid commands (spam) | Board processes sequentially, no crash |
| 12.7 | EEPROM corrupted/empty | Factory defaults applied |
| 12.8 | MCP2515 crystal mismatch | Init tries 8MHz then 16MHz |
| 12.9 | Command > 31 chars | Buffer overflow rejected (poison flag) |
| 12.10 | Invalid characters in command | Only a-z, A-Z, 0-9, :, -, _ accepted |
| 12.11 | (BT) HC-05 disconnects during command | USB still works, no crash |
| 12.12 | (BT) Send on BT while USB sending | Both channels independent |

---

## ESP32 Scenarios

## ESP32-1: First-Time Setup — FSD Only (esp32)

**User Story:** New user wires ESP32 DevKit + 1× MCP2515, connects via USB serial.

**Hardware:** ESP32 DevKit, MCP2515 (8 MHz crystal), USB cable

**Wiring:**

| MCP2515 Pin | ESP32 Pin | Function |
| ----------- | --------- | -------- |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | GPIO 15 (PIN_MCP2515_1_CS) | SPI chip select |
| INT | GPIO 34 (PIN_MCP2515_1_INT) | Hardware interrupt (input-only) |
| SCK | GPIO 18 (PIN_SPI_SCK) | SPI clock |
| MOSI | GPIO 23 (PIN_SPI_MOSI) | SPI MOSI |
| MISO | GPIO 19 (PIN_SPI_MISO) | SPI MISO |
| CAN-H | X179 pin 13 | FSD CAN high |
| CAN-L | X179 pin 14 | FSD CAN low |

| Step | Action | Expected Result | Status |
| ---- | ------ | -------------- | ------ |
| 1 | Wire MCP2515 to ESP32 per table | Physical wiring complete | — |
| 2 | Flash `esp32` via PlatformIO (COM4) | Upload successful | — |
| 3 | Open serial monitor at 115200 | Boot JSON received | — |
| 4 | Verify boot | `{"t":"boot","hw":"ESP32S_DevKit","can":"MCP2515_3x","drv":"arduino-mcp2515","busFsd":true,"busVehicle":false,"busBody":false,"wifiEnabled":false,"bleEnabled":false,...}` | — |
| 5 | Connect web dashboard via USB | Same Web Serial flow as Uno | — |
| 6 | MCP2515 init tries 8MHz then 16MHz | Crystal auto-detection | — |

**Pass Criteria:** ESP32 running FSD bus only, USB serial functional, no WiFi/BLE.

---

## ESP32-2: Full Bus Setup with X179 (esp32 + BUS_VEHICLE_ACTIVE=1 + BUS_BODY_ACTIVE=1)

**User Story:** User wires all 3× MCP2515 modules for full Tesla X179 connector coverage.

**X179 Connector Bus Mapping (hardcoded in config/esp32.h):**

| Bus Index | MCP2515 # | X179 Pins | CAN Bus Function | CS GPIO | INT GPIO |
| --------- | --------- | --------- | --------------- | ------- | -------- |
| 0 (BUS_FSD) | #1 | 13-14 | FSD / Autopilot | 15 | 34 |
| 1 (BUS_VEHICLE) | #2 | 9-10 | Vehicle Control | 27 | 35 |
| 2 (BUS_BODY) | #3 | 2-3 | Body Control | 26 | 33 |

**All 3 share SPI:** SCK=18, MOSI=23, MISO=19

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Wire all 3 MCP2515 modules per table | Three modules sharing SPI bus |
| 2 | Flash `esp32` with `BUS_VEHICLE_ACTIVE=1 BUS_BODY_ACTIVE=1` | Boot shows `busFsd:true, busVehicle:true, busBody:true` |
| 3 | Bus 0 (FSD) receives autopilot frames | 0x399, 0x3FD, 0x3F8 |
| 4 | Bus 1 (Vehicle) receives control frames | 0x273, 0x2F3, 0x333, 0x334 |
| 5 | Bus 2 (Body) receives body frames | 0x119 (window/vent), 0x284 (sentry), trunk ctrl |
| 6 | All 3 buses use hardware interrupts | ESP32 supports INT on GPIO 34, 35, 33 |
| 7 | FSD mods applied on Bus 0 | Handler dispatches by bus index |
| 8 | Vehicle commands sent on Bus 1 | Climate, charge, drive routed correctly |
| 9 | Body commands sent on Bus 2 | Window, sentry, trunk routed correctly |
| 10 | One bus init fails | Remaining buses still operate |

**ESP32 vs Uno dispatch:** ESP32 routes by bus index (`handleMessage` checks `bus` parameter), not by frame ID scanning. Bus 2 also uses hardware interrupts (unlike Uno polled D6).

---

## ESP32-3: WiFi AP + Dashboard (esp32_wifi)

**User Story:** User enables WiFi AP for wireless dashboard access.

**Firmware:** `esp32_wifi` — WiFi ON, BLE OFF

| Step | Action | Expected Result | Status |
| ---- | ------ | -------------- | ------ |
| 1 | Flash `esp32_wifi` firmware | Upload with `embed_html.py` pre-build | — |
| 2 | ESP32 creates AP "TeslaCANModder" | SSID visible on phone/laptop | — |
| 3 | Connect to AP (password: `teslacan123`, channel 6) | DHCP assigns IP (gateway: 192.168.4.1) | — |
| 4 | Open <http://192.168.4.1> | Embedded HTML dashboard loads | — |
| 5 | Dashboard shows system status cards | Hardware, CAN stats, WiFi status, settings | — |
| 6 | Send command via dashboard button | POST `/api/command` → ACK | — |
| 7 | Check WiFi status card | Mode: AP, IP: 192.168.4.1, SSID: TeslaCANModder | — |
| 8 | USB serial still works simultaneously | Both channels independent | — |

**WiFi REST API Endpoints:**

| Method | Path | Purpose |
| ------ | ---- | ------- |
| GET | `/` | HTML dashboard (PROGMEM) |
| GET | `/api/ping` | `{"t":"pong","v":1}` |
| GET | `/api/status` | Full state JSON (variant, FSD, nag, profile, features, hardware, CAN stats) |
| POST | `/api/command` | Execute command: `{"cmd":"fsd:on"}` → returns updated status |
| GET | `/api/disable` | Emergency kill: FSD off, summon stop |
| GET | `/api/wifi/status` | WiFi mode, SSID, IP, RSSI/clients, gateway, MAC |
| POST | `/api/wifi/config` | Switch WiFi mode: `{"mode":"ap"\|"sta","ssid":"...","password":"..."}` |

**All endpoints return CORS headers** (`Access-Control-Allow-Origin: *`) with OPTIONS preflight support.

---

## ESP32-4: WiFi AP → STA Mode Switching

**User Story:** User switches from AP mode to home STA network for remote access.

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Start in AP mode (default) | 192.168.4.1 accessible |
| 2 | POST `/api/wifi/config` with `{"mode":"sta","ssid":"HomeWiFi","password":"pass123"}` | Board disconnects AP |
| 3 | ESP32 connects to home router | STA mode, new IP from router DHCP |
| 4 | STA connection timeout = 15s | Falls back to AP on failure |
| 5 | Access dashboard at new IP | Same dashboard on home network |
| 6 | POST `/api/wifi/config` with `{"mode":"ap"}` | Board creates AP again |
| 7 | WiFi config persists in NVS | `tcm_wifi` namespace: mode, SSID, password |
| 8 | Power cycle | Board boots in saved WiFi mode |

---

## ESP32-5: BLE Connection (esp32_ble)

**User Story:** User connects to ESP32 via BLE for wireless serial-like access. BLE works with iOS and Android.

**Firmware:** `esp32_ble` — BLE ON, WiFi OFF

**BLE Service:** Nordic UART Service (NUS)

| UUID | Direction | Purpose |
| ---- | --------- | ------- |
| `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | — | Service UUID |
| `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write (phone → device) | RX: Send commands |
| `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Notify (device → phone) | TX: Receive responses |

| Step | Action | Expected Result | Status |
| ---- | ------ | -------------- | ------ |
| 1 | Flash `esp32_ble` firmware | Boot shows BLE capability | — |
| 2 | Open BLE scanner (nRF Connect, LightBlue) | "TeslaCANModder" device visible | — |
| 3 | Connect to device | BLE connection established | — |
| 4 | Subscribe to TX characteristic (6E400003) | Status notifications arrive | — |
| 5 | Write `ping` to RX characteristic (6E400002) | `{"t":"pong","v":1}` notified back | — |
| 6 | Write `fsd:on` | ACK notification, FSD enabled | — |
| 7 | BLE and USB serial simultaneous | Both channels work independently | — |
| 8 | Disconnect BLE | Board continues, auto-restarts advertising | — |
| 9 | Reconnect BLE | Resume receiving notifications | — |

**BLE vs HC-05 (Uno) Comparison:**

| Feature | ESP32 BLE (NimBLE) | Uno HC-05 (SPP) |
| ------- | ----------------- | --------------- |
| Protocol | BLE GATT (NUS) | Bluetooth Classic SPP |
| iOS support | ✓ | ✗ (iOS blocks SPP) |
| Android support | ✓ | ✓ |
| Encryption | BLE pairing | None |
| Range | ~30m | ~10m |
| Power | Low power | Higher power |
| Baud rate | N/A (packet-based) | 9600 baud |
| Buffer | 256-byte ring buffer | SoftwareSerial buffer |
| Runtime toggle | Yes (via WiFi API) | No (hardware always on) |
| TX power | ESP_PWR_LVL_P9 | Fixed |

---

## ESP32-6: WiFi + BLE Combined (esp32_wifi_ble)

**User Story:** Full-featured ESP32 with all 3 connectivity options: USB Serial + WiFi + BLE.

**Firmware:** `esp32_wifi_ble` — WiFi ON, BLE ON

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Flash `esp32_wifi_ble` | Boot shows WiFi + BLE + active buses |
| 2 | WiFi AP active | "TeslaCANModder" AP visible |
| 3 | BLE advertising | "TeslaCANModder" in BLE scanner |
| 4 | USB serial active | 115200 baud, boot JSON received |
| 5 | All 3 channels accept commands | Same command set, same responses |
| 6 | Command from WiFi affects BLE status | State is shared across all channels |
| 7 | Toggle BLE off via WiFi dashboard | BLE stops advertising, disconnects |
| 8 | GET `/api/ble/status` | `{"enabled":false,"connected":false,"deviceName":"TeslaCANModder"}` |
| 9 | POST `/api/ble/config` `{"enabled":true}` | BLE restarts, device visible again |
| 10 | BLE state persists in NVS | `tcm_ble` namespace |

**BLE REST API (WiFi+BLE firmware only):**

| Method | Path | Purpose |
| ------ | ---- | ------- |
| GET | `/api/ble/status` | `{"enabled":bool,"connected":bool,"deviceName":"TeslaCANModder"}` |
| POST | `/api/ble/config` | `{"enabled":true\|false}` — starts/stops BLE at runtime, saves to NVS |

---

## ESP32-7: FSD + Nag Configuration

**User Story:** Same feature set as Uno, accessible via USB, WiFi, or BLE.

| Step | Action (any channel) | Expected Result |
| ---- | ------------------- | -------------- |
| 1 | `fsd:on` | ACK, FSD enabled |
| 2 | `nag:on` | ACK, nag suppression ON |
| 3 | `profile:2` | Profile pinned to 2 |
| 4 | `profile:auto` | Profile follows stalk |
| 5 | (HW4) `isa:on` | ISA chime suppressed |
| 6 | (HW3) `offset:60` | Speed offset 60% |
| 7 | `status` | All settings in JSON |
| 8 | Power cycle | Settings restored from NVS |

**NVS Persistence:** Namespace `tcm`, magic `0xCA`, version `0x02`. Keys: `variant`, `fsd`, `nag`, `sp`, `spPin`, `offset`, `offPin`, `isa`.

---

## ESP32-8: Vehicle Controls

**Same command set as Uno** — requires `BUS_VEHICLE_ACTIVE=1` and/or `BUS_BODY_ACTIVE=1` for vehicle/window/sentry/climate/charge/drive commands to be compiled in.

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Wait for frame caches to populate | `hasCtrl`, `hasClimate`, `hasCharge`, `hasDrive` from live CAN |
| 2 | `unlock` via WiFi dashboard | 30× 0x273 burst on Bus 1 (Vehicle) |
| 3 | `frunk:open` | 50× 0x273 burst |
| 4 | `climate:keep` | 0x2F3 modified, sent on Bus 1 |
| 5 | `vent:open` | 0x119 sent on Bus 2 (Body) |
| 6 | `sentry:on` | 0x284 sent on Bus 2 (Body) |
| 7 | `pedal:sport` | 0x334 modified, sent on Bus 1 |
| 8 | Send command on 1-CAN firmware | Vehicle commands not available (not compiled) |

**ESP32 Bus Routing:**

| Command Group | Target Bus | CAN ID |
| ------------- | --------- | ------ |
| FSD, nag, profile, ISA | Bus 0 (FSD) | 0x399 (intercepted) |
| lock, unlock, mirror, seat, power | Bus 1 (Vehicle) | 0x273 |
| climate | Bus 1 (Vehicle) | 0x2F3 |
| charge | Bus 1 (Vehicle) | 0x333 |
| drive, pedal, regen | Bus 1 (Vehicle) | 0x334 |
| window vent | Bus 2 (Body) | 0x119 |
| sentry | Bus 2 (Body) | 0x284 |
| trunk, frunk | Bus 2 (Body) | trunk ctrl ID |

---

## ESP32-9: Summon

**Same as Uno**, but summon only functions when `BUS_VEHICLE_ACTIVE=1`. Burst frames sent on Bus 1 (Vehicle) using cached 0x273.

---

## ESP32-10: CAN Stream & Debugging

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | `stream:on` (via any channel) | CAN frames streamed as JSON |
| 2 | Frames show bus index (0, 1, 2) | All 3 buses visible |
| 3 | `can:raw:on` | All buses pass-through (filters cleared) |
| 4 | `can:raw:off` | Per-bus filters re-applied |
| 5 | WiFi command while streaming | No delay on frame processing |

---

## ESP32-11: Powerbank Deployment

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Configure via WiFi dashboard | Settings saved to NVS |
| 2 | Disconnect laptop | Board still powered |
| 3 | Connect powerbank | Board reboots, loads NVS settings |
| 4 | Place in car | Board running standalone |
| 5 | Drive — CAN frames flow | All mods active |
| 6 | Park, walk away | CAN bus silent, standby after 10s |
| 7 | WiFi AP still active | Can reconnect phone to check status |
| 8 | BLE still advertising | Can connect phone BLE to check |
| 9 | Return, open door | CAN wakes, auto-recovery |
| 10 | All mods resume | No user interaction needed |

---

## ESP32-12: Power Cycle Recovery

| Step | Action | Expected Result |
| ---- | ------ | -------------- |
| 1 | Board running normally | All features active |
| 2 | Power loss (2 seconds) | Board shuts down |
| 3 | Power restored | Board reboots |
| 4 | NVS settings restored | FSD/Nag/Profile match pre-disconnect |
| 5 | WiFi AP restarts | Dashboard accessible at 192.168.4.1 |
| 6 | BLE re-advertises | Device visible in scanner |
| 7 | CAN frames resume | Mods applied immediately |
| 8 | Frame caches rebuild | hasCtrl/hasClimate/hasCharge/hasDrive from live CAN |

---

## ESP32-13: Error Handling

| Case | Trigger | Expected Result |
| ---- | ------- | -------------- |
| 13.1 | Send command without CAN init | Error response |
| 13.2 | Vehicle cmd without 0x273 cache | Error: "Waiting for 0x273 frame" |
| 13.3 | Climate cmd without 0x2F3 cache | Error: "Need 0x2F3" |
| 13.4 | Unknown command | Error: "Unknown command" |
| 13.5 | WiFi client during active CAN | No frame processing delay |
| 13.6 | BLE and WiFi both active | All 3 I/O paths work simultaneously |
| 13.7 | WiFi AP→STA switch fails (timeout 15s) | Falls back to AP mode |
| 13.8 | NVS corrupted/empty | Factory defaults applied |
| 13.9 | MCP2515 crystal mismatch | Init tries 8MHz then 16MHz |
| 13.10 | 1-CAN firmware, vehicle command sent | Command not available (not compiled) |
| 13.11 | BLE toggle off via REST, reconnect attempt | Device not found in scanner |
| 13.12 | Command > 31 chars | Buffer overflow rejected |
| 13.13 | Invalid chars in REST command body | Same validation as serial (a-z, A-Z, 0-9, :, -, _) |
| 13.14 | CORS preflight request | OPTIONS returns proper headers |

---

## Cross-Board Scenarios

## CROSS-1: Variant Behavior Consistency

**Same variant behavior on both boards:**

| Feature | HW4 | HW3 | Legacy |
| ------- | --- | --- | ------ |
| FSD enable/disable | ✓ | ✓ | ✓ |
| Nag suppression | ✓ | ✓ | ✓ |
| Speed profile | Auto/Pin (0-3) | Auto/Pin (0-3) | 0-2 only |
| Speed offset | — | ✓ (0-100%) | — |
| ISA chime suppress | ✓ | — | — |
| Summon | ✓ | ✓ | — |

**Handler dispatch differences:**

- Uno: `handleMessage` routes Bus 0 to variant handler, Bus 1+ for frame caching
- ESP32: `handleMessage` routes by bus index (Bus 0=FSD, Bus 1=Vehicle, Bus 2=Body)

---

## CROSS-2: Command Parity

All commands work identically across all I/O channels:

| Channel | Board | Baud/Protocol | Simultaneous |
| ------- | ----- | ------------- | ----------- |
| USB Serial | Both | 115200 baud, JSON | Always |
| HC-05 BT | Uno (`uno_bt`) | 9600 baud SPP, JSON | Yes (with USB) |
| BLE (NimBLE) | ESP32 (`*_ble`) | BLE NUS packets, JSON | Yes (with USB + WiFi) |
| WiFi REST | ESP32 (`*_wifi`) | HTTP REST, JSON | Yes (with USB + BLE) |
| WiFi Dashboard | ESP32 (`*_wifi`) | HTML UI → REST | Yes (with all) |

**Full Command Reference:**

| Command | Domain | Cached Frame Required |
| ------- | ------ | -------------------- |
| `ping` | System | — |
| `status` | System | — |
| `stream:on` / `stream:off` | System | — |
| `can:raw:on` / `can:raw:off` | System | — |
| `fsd:on` / `fsd:off` / `fsd:toggle` | FSD | — |
| `nag:on` / `nag:off` / `nag:toggle` | FSD | — |
| `profile:0-3` / `profile:auto` | FSD | — |
| `offset:0-100` / `offset:auto` | FSD (HW3) | — |
| `isa:on` / `isa:off` | FSD (HW4) | — |
| `summon:forward` / `summon:reverse` / `summon:stop` | Summon | `hasCtrl` (0x273) |
| `variant:hw4` / `variant:hw3` / `variant:legacy` | System | — |
| `lock` / `unlock` / `lock:child` | Vehicle | `hasCtrl` (0x273) |
| `frunk:open` / `frunk:close` / `frunk` | Trunk | `hasCtrl` (0x273) |
| `trunk:open` / `trunk:close` / `trunk` | Trunk | `hasCtrl` (0x273) |
| `mirror:fold` / `mirror:unfold` / `mirror:heat` | Mirror | `hasCtrl` (0x273) |
| `light:fog:front` / `light:fog:rear` / `light:highbeam:auto` | Light | `hasCtrl` (0x273) |
| `wiper:off` / `wiper:1` / `wiper:2` | Wiper | `hasCtrl` (0x273) |
| `seat:fl:0-3` / `seat:fr:0-3` | Seat | `hasCtrl` (0x273) |
| `maindisplay:<0-127>` | Display | `hasCtrl` (0x273) |
| `power` | Power | `hasCtrl` (0x273) |
| `window:vent:open` / `window:vent:close` | Window | — (3-CAN required) |
| `sentry:on` / `sentry:off` | Sentry | — (3-CAN required) |
| `climate:keep` / `climate:off` | Climate | `hasClimate` (0x2F3) |
| `charge:start` / `charge:stop` / `charge:port` | Charge | `hasCharge` (0x333) |
| `pedal:sport` / `pedal:chill` / `pedal:std` | Drive | `hasDrive` (0x334) |

---

## CROSS-3: Persistence Parity

| Field | Uno (EEPROM) | ESP32 (NVS) |
| ----- | ----------- | ----------- |
| Magic | 0xCA at addr 0 | `magic` key = 0xCA |
| Version | 0x02 at addr 1 | `ver` key = 0x02 |
| Variant | byte | `variant` key |
| FSD enabled | byte | `fsd` key |
| Nag suppress | byte | `nag` key |
| Speed profile | byte | `sp` key |
| Profile override | byte | `spPin` key |
| Speed offset | byte | `offset` key |
| Offset override | byte | `offPin` key |
| ISA chime | byte | `isa` key |
| WiFi config | — | `tcm_wifi` namespace (mode, SSID, password) |
| BLE config | — | `tcm_ble` namespace (enabled) |

---

## CROSS-4: CAN Health & Standby

Both boards implement the same CAN health monitoring:

| Constant | Value | Purpose |
| -------- | ----- | ------- |
| `CAN_TIMEOUT_MS` | 10,000 ms | No frames → enter standby |
| `CAN_REINIT_INTERVAL` | 5,000 ms | Retry MCP2515 init in standby |
| `LED_STANDBY_INTERVAL` | 2,000 ms | LED slow-blink in standby |

**Standby behavior:** MCP2515 listen-only, reduced polling, LED slow-blink. Auto-recovery when CAN traffic resumes.

---

## CROSS-5: MCP2515 Hardware Filter Mapping

Both boards use MCP2515 hardware filters (RXF0-5, MASK0/MASK1) set per variant and per bus.

| Bus | Uno Filter Source | ESP32 Filter Source |
| --- | ---------------- | ----------------- |
| 0 | Variant FSD IDs (0x399, etc.) | Same — FSD bus only |
| 1 | Vehicle IDs (0x273, 0x2F3, 0x333, 0x334) | Same — Vehicle bus only |
| 2 | (if 3-CAN) Body IDs | Same — Body bus only |
| Raw mode | All filters cleared | All filters cleared |

`applyFilters(State&)` is called on variant change, raw mode toggle, and CAN reinit.
