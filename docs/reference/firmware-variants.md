---
title: Firmware Variants
title_tr: Firmware Varyantları
description: Build configurations for ESP32 and its connectivity options
category: reference
folder: reference
tags: [firmware, variants, build]
order: 8
icon: 💾
---

# Firmware Variants

TeslaCANModder ships a full PlatformIO build matrix for the ESP32 instead of a small set of fixed variants. Every connectivity option, every CAN bus, and the MCP2515 crystal frequency compose into the environment name.

## Naming Convention

```
esp32[_wifi][_ble][_chassis][_vehicle][_body][_8mhz|_16mhz]
```

All segments are optional except the clock suffix — every env has an explicit `_8mhz` or `_16mhz` tail. Order is fixed (connectivity first, buses next, clock last).

Default env is **`esp32_chassis_8mhz`** (chassis-only, USB serial, 8 MHz crystal). The release matrix builds ~44 combinations covering the realistic permutations.

## Connectivity Segments

| Segment | Build flag            | Purpose                              |
| ------- | --------------------- | ------------------------------------ |
| `_wifi` | `BOARD_ENABLE_WIFI=1` | REST API + dashboard (AP/STA)        |
| `_ble`  | `BOARD_ENABLE_BLE=1`  | NimBLE Nordic UART + gamepad central |

Omit both for a USB-serial-only build.

## CAN Bus Segments (all opt-in)

| Segment    | Build flag             | Pins / connector | Function                                                                            |
| ---------- | ---------------------- | ---------------- | ----------------------------------------------------------------------------------- |
| `_chassis` | `BUS_CHASSIS_ACTIVE=1` | X179 pins 13–14  | Chassis CAN — **required for DAS injection**                                        |
| `_vehicle` | `BUS_VEHICLE_ACTIVE=1` | X179 pins 9–10   | Vehicle Control (mirror, lock, climate, charge, drive, seat, wiper, display, power) |
| `_body`    | `BUS_BODY_ACTIVE=1`    | X179 pins 2–3    | Body Control (window, sentry, trunk)                                                |

> No bus is hardcoded active. A build without `_chassis` runs as a passive sniffer / logger on whatever buses are enabled — DAS injection is disabled in that mode because the steering and braking ECUs only listen on Chassis CAN.

## Other Build Flags

| Flag                  | Values  | Description                                                                                                 |
| --------------------- | ------- | ----------------------------------------------------------------------------------------------------------- |
| `BOARD_CAN_CLOCK_MHZ` | 8 or 16 | MCP2515 crystal frequency — must match the modules wired up. Selected by the `_8mhz` / `_16mhz` env suffix. |

## Building with PlatformIO

```bash
# Default — chassis only, USB serial, 8 MHz
pio run -e esp32_chassis_8mhz

# Full I/O — WiFi + BLE + all four buses on 8 MHz crystals
pio run -e esp32_wifi_ble_chassis_vehicle_body_8mhz

# Same combo on 16 MHz crystals
pio run -e esp32_wifi_ble_chassis_vehicle_body_16mhz

# Passive sniffer — no chassis, vehicle + body only
pio run -e esp32_vehicle_body_8mhz

# Smoke the full release artifact matrix locally
npm run smoke:firmware:release-matrix

# Build and upload the default env
pio run -e esp32_chassis_8mhz -t upload

# Run native tests
pio test -e native
```

The smoke sweep mirrors the GitHub release matrix, rebuilds each environment, and copies the merged `.bin` outputs to `firmware/build/release-matrix-smoke/` together with a `report.json` manifest.

Release asset names follow the env name 1:1 — e.g. `esp32_chassis_8mhz.bin`, `esp32_wifi_ble_chassis_vehicle_body_16mhz.bin`. To list every shipped env, see `firmware/platformio.ini` (each `[env:...]` block is a release artifact).

## Flashing via Web UI

1. Go to the **Flasher** tab in the web UI
2. Select connectivity (USB / WiFi / BLE)
3. Toggle which CAN buses to enable (Chassis / Vehicle / Body)
4. Pick the MCP2515 crystal clock (8 MHz or 16 MHz)
5. Click **Build & Download** or **Flash via USB**
6. Monitor the console for progress and boot messages

> Vehicle controls (mirror, lock, climate, charge, drive, seat heating, wiper, display, power) require `BUS_VEHICLE_ACTIVE=1`. Body controls (window, sentry, trunk) require `BUS_BODY_ACTIVE=1`. DAS injection (FSD, nag suppression, steering / braking spoofs) requires `BUS_CHASSIS_ACTIVE=1`.
