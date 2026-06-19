# Tesla CAN Modder Firmware

Active PlatformIO firmware for ESP32-S DevKit with MCP2515 CAN buses, optional WiFi REST API, and optional BLE.

## Supported Environments

| Environment      | Connectivity            |
| ---------------- | ----------------------- |
| `esp32`          | USB serial              |
| `esp32_wifi`     | USB serial + WiFi       |
| `esp32_ble`      | USB serial + BLE        |
| `esp32_wifi_ble` | USB serial + WiFi + BLE |
| `native`         | Host-native unit tests  |

## Build

```powershell
cd firmware
.\.pio.ps1 run -e esp32
.\.pio.ps1 run -e esp32_wifi
.\.pio.ps1 run -e esp32_ble
.\.pio.ps1 run -e esp32_wifi_ble
```

Upload to a board:

```powershell
.\.pio.ps1 run -e esp32_wifi_ble -t upload
```

Enable additional buses with build flags:

```powershell
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_CHASSIS_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\.pio.ps1 run -e esp32_wifi_ble
```

## Validation

```powershell
.\.pio.ps1 test -e native
.\.pio.ps1 run -e esp32_wifi
.\.pio.ps1 run -e esp32_wifi_ble
```

## Hardware Model

Primary target:

- ESP32-S DevKit
- 1-3 MCP2515 modules with 8 MHz crystals
- Tesla X179 connector

Lane mapping:

| Bus     | X179 Pins | Purpose                 |
| ------- | --------- | ----------------------- |
| Chassis | 13-14     | Chassis / autopilot CAN |
| Vehicle | 9-10      | Vehicle control CAN     |
| Body    | 2-3       | Body control CAN        |

See `../docs/guides/hardware-setup.md` for wiring details.

## Runtime Capabilities

- Variant switching for `hw4`, `hw3`, and `legacy`
- FSD enable/nag/profile/offset/ISA control paths
- Summon, climate, charge, window, sentry, lock, mirror, seat, light, and drive command support where the active buses and variant allow it
- JSON status and frame streaming over USB, WiFi, and BLE
- OTA-aware TX pause and persisted settings in NVS

## Layout

| Path                       | Role                                                                     |
| -------------------------- | ------------------------------------------------------------------------ |
| `lib/core/`                | shared types, config, persistence, log ring, CAN plumbing, and driver    |
| `lib/core/can/`            | MCP2515 bus init, frame TX/RX, ring buffer, recorder, health             |
| `lib/vehicle/can/feature/` | feature command handlers and frame mutation helpers                      |
| `lib/vehicle/can/handler/` | variant-specific frame handlers (HW3, HW4, legacy) and dispatch          |
| `lib/vehicle/ble/`         | Tesla BLE protocol (key, session, vcsec, carserver)                      |
| `lib/io/`                  | USB serial, WiFi REST API/dashboard, and BLE transport layers            |
| `lib/client/`              | REST API, dashboard, command dispatch, gamepad                           |
| `lib/transport/`           | legacy alias tree (uses `core/` types); kept for staged migration        |
| `lib/interface/`           | legacy alias tree (umbrella for `transport/`); kept for staged migration |
| `src/esp32/main.cpp`       | ESP32 firmware entry point                                               |
| `test/`                    | native PlatformIO regression suites                                      |

> The `lib/transport/` and `lib/interface/` trees mirror content under `lib/core/can/` and `lib/vehicle/can/handler/`. They are compiled in the single-TU build (via `lib_ldf_mode = deep+`) and remain in lockstep with their canonical counterparts. Consolidation is tracked as a follow-up.

The shipping firmware lives here. `hardware/` is reference material only and is not the active release target.
