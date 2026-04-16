# TeslaCANModder

[![CI](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml/badge.svg)](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml)

Browser-based control and monitoring stack for Tesla CAN bus modification using Arduino Uno or ESP32-S DevKit.

## Quick Start

### Hardware
```powershell
cd hardware
.\.pio.ps1 run -e uno          # Arduino Uno, serial only
.\.pio.ps1 run -e uno_bt        # Arduino Uno + HC-05 Bluetooth
.\.pio.ps1 run -e esp32         # ESP32, serial only
.\.pio.ps1 run -e esp32_wifi    # ESP32 + WiFi REST API
.\.pio.ps1 run -e esp32_ble     # ESP32 + BLE
.\.pio.ps1 run -e esp32_wifi_ble # ESP32 + WiFi + BLE
.\.pio.ps1 run -e uno -t upload  # Flash to board
```

CAN bus selection is controlled via build flags (FSD always on):
```powershell
# Enable Vehicle + Body buses alongside FSD
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_FSD_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\.pio.ps1 run -e esp32_wifi
```

### Web UI
```bash
cd web
npm install
npm run dev
```

Open http://localhost:5173 and click "Connect USB"

## What's Included

- **Hardware Firmware** - Arduino Uno + ESP32-S firmware with per-bus feature gating
- **Web UI** - React dashboard with Web Serial API, CAN frame decoder
- **Docker Support** - Build firmware and run web UI in containers

## Hardware

- **Arduino Uno R3** (CH340 or ATmega16U2) — up to 3× MCP2515, optional HC-05 BT
- **ESP32-S DevKit** — up to 3× MCP2515, built-in WiFi + BLE
- MCP2515 CAN module (8 MHz crystal) + TJA1050 transceiver per bus
- 9V-36V to 5V converter
- Tesla X179 connector

### X179 CAN Bus Lanes

| Bus | X179 Pins | Build Flag | Default |
|---- | --------- | ---------- | ------- |
| FSD (Bus 0) | 13-14 | `BUS_FSD_ACTIVE` | ON |
| Vehicle (Bus 1) | 9-10 | `BUS_VEHICLE_ACTIVE` | OFF |
| Body (Bus 2) | 2-3 | `BUS_BODY_ACTIVE` | OFF |

## Features

### Runtime Variant Switching
- HW4 (2026.2.3+) with ISA chime control
- HW3 with speed offset control
- Legacy for pre-HW3 vehicles

### Controls (All OFF by default, user enables via web)
- FSD enable/disable/toggle
- Nag suppression
- Speed profile (Chill / Normal / Hurry / Max / Sloth)
- Speed offset (HW3: 0-100%)
- ISA chime suppression (HW4)
- ASS (Autopark Summon System) - Forward/Reverse/Stop
- CAN frame streaming with per-bus lane labels
- CAN frame decoder (577 known Tesla frames)
- Per-bus vehicle controls (Vehicle bus: mirror/lock/climate/charge/drive, Body bus: window/sentry)

## Memory Usage

| Build | RAM | Flash |
|------ | --- | ----- |
| USB + Bluetooth | 1558 bytes (76%) | 11150 bytes (35%) |
| USB only | 1378 bytes (67%) | 9668 bytes (30%) |

## Protocol

JSON messages over serial (115200 baud):
- `boot` - Board initialization
- `status` - State updates (every 500ms)
- `frame` - CAN frames (when streaming)
- `ack` / `error` - Command responses
- `pong` - Ping response

## Commands

### Core FSD Controls
```
ping, status
variant:hw4, variant:hw3, variant:legacy
fsd:on, fsd:off, fsd:toggle
nag:on, nag:off, nag:toggle
profile:0-4 (or sp:0-4)
offset:0-100 (HW3 only)
isa-chime:on, isa-chime:off, isa-chime:toggle (HW4 only)
stream:on, stream:off
can:raw:on, can:raw:off
```

### ASS Summon
```
summon, summon:forward, summon:fwd
summon:reverse, summon:rev
summon:stop
```

### Vehicle Control (0x273 UI_vehicleControl)
```
# Mirror
mirror:fold, mirror:unfold, mirror:heat
mirror:autofold, mirror:dip

# Lock
lock, unlock, lock:child

# Trunk/Frunk
frunk, frunk:open, frunk:close
trunk, trunk:open, trunk:close
glovebox, horn

# Lighting (toggle switches)
light:fog:front, light:fog:rear
light:highbeam:auto, light:ambient, light:home
light:dome:off, light:dome:on, light:dome:auto

# Wiper
wiper:off, wiper:1, wiper:2, wiper:3

# Seat Heating (0-3: off/low/med/high)
seat:fl:0, seat:fl:1, seat:fl:2, seat:fl:3
seat:fr:0-3, seat:rl:0-3, seat:rr:0-3, seat:rc:0-3

# Display (main center screen, 0-127)
maindisplay:0-127

# Power (vehicle state control)
power:acc:on, power:acc:off, power:off, power:ready
```

### Advanced Features
```
# Window Vent (0x119)
window:vent:open, vent:open
window:vent:close, vent:close

# Sentry Mode (0x284)
sentry:on, sentry:off

# Climate Control (0x2F3 - requires frame caching)
climate:keep, climate:off

# Charge Control (0x333 - requires frame caching)
charge:start, charge:stop
charge:port, chargeport

# Drive Configuration (0x334 - requires frame caching)
# Pedal Response
pedal:standard, pedal:std, pedal:chill, pedal:sport

# Regenerative Braking (0-200)
regen:off, regen:low, regen:std, regen:max

# Stop Mode
stop:creep, stop:roll, stop:hold
```

## Browser Support

- ✅ Desktop Chrome/Edge - Full support (flashing + control)
- ✅ Android Chrome - Runtime control via Bluetooth
- ⚠️ Other browsers - Guide mode only

## Workspace

This is an npm workspace monorepo:

```
packages/protocol   — shared types, commands, decoder, parser (@teslacanmodder/protocol)
hardware            — PlatformIO ESP32/Arduino firmware
web                 — React + Vite + TypeScript dashboard
mobile              — React Native + Expo mobile app
tools               — CLI debug utilities
```

```bash
npm install          # install all workspaces
npm run test:all     # run all tests (protocol + web + mobile + tools)
```

## Testing

| Layer | Runner | Tests |
|------ | ------ | ----- |
| Firmware | PlatformIO Unity | 178 |
| Protocol | Jest (ESM) | 102 |
| Web | Vitest + Testing Library | 63 |
| Mobile | Jest + Testing Library/RN | 138 |
| **Total** | | **481** |

## CI

GitHub Actions runs on push to `main` and all PRs:
- **firmware** — PlatformIO native tests
- **protocol** — shared package tests
- **web** — type-check + test + build
- **mobile** — Jest tests
- **tools** — CLI tests
- **docker** — docker compose build

## Documentation

- `hardware/README.md` - Firmware details
- `web/README.md` - Web UI details
- `docs/WIFI_BOARD_GUIDE.md` - ESP32 WiFi dashboard setup

## License

GPL-3.0
