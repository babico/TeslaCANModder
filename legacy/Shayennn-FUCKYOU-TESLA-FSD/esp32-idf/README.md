# ESP32 Firmware

Pure ESP-IDF v6.0 firmware for the `ESP32-WROOM-32` Tesla FSD CAN mod.

This is the primary ESP32 implementation in the repository. It supersedes the older
`esp32/esp32.ino` sketch and is designed around a deterministic CAN fast path first,
with Wi-Fi, web UI, OTA, and diagnostics kept off the hot path.

## Current Feature Set

- Core 1 CAN fast path using the ESP-IDF v6.0 TWAI callback API
- Shared production handler logic reused across firmware and desktop tests
- Open Wi-Fi AP named `TeslaFSD`
- AP IP `192.168.4.1/24`
- DHCP router and DNS offers explicitly disabled
- First-boot password setup and challenge-response login
- Ephemeral bearer session token after login
- Password recovery arm flow from the web UI
- Minimal live status panel and bounded CAN log viewer
- Dual-slot OTA updates from the web UI
- Flash coredumps and separate release/debug config defaults

## Repository Layout

- `main/esp32_app.cpp`: app entry point and subsystem wiring
- `main/can_fast_path.*`: RX ISR -> ring buffer -> Core 1 CAN processing task
- `main/can_logger.*`: bounded in-memory CAN log used by the web UI
- `main/auth.*`: password storage, challenge generation, session validation
- `main/wifi_ap.*`: open AP setup with no advertised gateway or DNS
- `main/web_server.*`: web UI and JSON API
- `main/ota.*`: OTA write/finalize/reboot handling
- `../shared/vehicle_logic.h`: shared production handler logic

## Hardware Defaults

| Item | Value |
|---|---|
| Target | `ESP32-WROOM-32` |
| Flash size | `4 MB` |
| CAN RX pin | `GPIO27` |
| CAN TX pin | `GPIO26` |
| CAN bitrate | `500000` |
| Status LED | `GPIO2` |
| AP SSID | `TeslaFSD` |
| AP auth | Open |
| AP channel | `6` |
| AP max clients | `4` |
| AP IP | `192.168.4.1/24` |

## Build Requirements

- ESP-IDF `v6.0`
- Python environment activated through your IDF install

Example activation:

```bash
source ~/.espressif/tools/activate_idf_v6.0.sh
```

## Build

First-time setup:

```bash
cd esp32-idf
idf.py set-target esp32
idf.py build
```

Release build:

```bash
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release.defaults" build
```

Debug build:

```bash
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug.defaults" build
```

Current build output is comfortably inside the OTA slot size, with roughly 55% free
space in the smallest application partition.

## Flash And Monitor

```bash
PORT=/dev/cu.usbserial-XXXX
idf.py -p "$PORT" flash monitor
```

Erase first when you want a clean first-boot password setup flow:

```bash
idf.py -p "$PORT" erase-flash flash monitor
```

## Runtime Architecture

The fast path is intentionally split into two stages:

1. The TWAI RX callback copies the frame into a fixed-size ring buffer and notifies a task.
2. A dedicated CAN task pinned to Core 1 pops frames, runs shared handler logic, and performs
   the immediate transmit attempt.

Important constraints kept in the hot path:

- No dynamic allocation in the fast path
- No virtual dispatch in the fast path
- No HTTP, Wi-Fi, logging UI, or OTA work in the fast path
- Fail-safe latch after repeated TX failures or bus-off

## Auth And Web Flow

`/` serves a single-page web UI with:

- First-boot password setup
- Login via challenge-response
- Handler selection
- Basic runtime counters
- Minimal decoded signal/status panel
- Bounded CAN log viewer
- OTA upload form

Public API endpoints:

- `GET /api/status`
- `GET /api/challenge`
- `POST /api/login`
- `POST /api/setup`
- `POST /api/recovery`

Authenticated API endpoints:

- `POST /api/handler`
- `GET /api/can_log`
- `POST /api/ota`

The browser login flow is:

1. Fetch `/api/challenge`
2. SHA-256 the password in the browser
3. SHA-256 the stored password hash plus the current challenge
4. POST the hex response to `/api/login`
5. Reuse the returned token as `Authorization: Bearer <hex>`

## OTA And Crash Support

- Dual OTA slots: `ota_0` and `ota_1`
- Delayed reboot after OTA response completes
- Flash coredumps enabled in both build profiles
- Panic backtraces, watchdogs, and stack protection enabled through sdkconfig defaults

## Partition Layout

| Partition | Offset | Size |
|---|---|---|
| `nvs` | `0x9000` | `0x5000` |
| `otadata` | `0xE000` | `0x2000` |
| `ota_0` | `0x10000` | `0x1E0000` |
| `ota_1` | `0x1F0000` | `0x1E0000` |
| `coredump` | `0x3D0000` | `0x30000` |

## Validation

Use `HARDWARE_VALIDATION.md` for the full bring-up and verification procedure, including:

- serial port discovery
- clean flash/build/monitor commands
- expected boot logs
- AP and DHCP validation
- browser and curl auth flows
- OTA verification
- password recovery verification
- CAN log and decoded-signal checks
- coredump retrieval

## Notes

- Desktop tests in `../test` compile the same shared production handler logic used here.
- The decoded signal panel is derived from the recent CAN log snapshot, not from a separate
  always-live signal cache.
- Hardware validation is still required for the AP, OTA, recovery, and real-bus latency paths.
