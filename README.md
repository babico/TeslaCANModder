# Tesla CAN Modder

[![CI](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml/badge.svg)](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml)

Tesla CAN firmware, client, protocol, and diagnostics tooling centered on ESP32-S DevKit hardware and the Tesla X179 connector.

## Overview

The active stack is split into four maintained areas:

| Area                 | Purpose                                                                              |
| -------------------- | ------------------------------------------------------------------------------------ |
| `firmware/`          | PlatformIO ESP32 firmware with 1-3 MCP2515 buses, WiFi REST API, and BLE             |
| `client/`            | Expo client for web, iOS, and Android, including the browser flasher and in-app docs |
| `packages/protocol/` | Shared protocol types, commands, decoder data, and parsing helpers                   |
| `tools/`             | Debug CLI and serial-to-HTTP bridge for bench work and validation                    |

The documentation screen now renders raw markdown directly from `docs/`. There is no generated TypeScript docs bundle anymore.

## Quick Start

Install workspace dependencies:

```bash
npm install
```

Build firmware locally:

```powershell
cd firmware
.\.pio.ps1 run -e esp32
.\.pio.ps1 run -e esp32_wifi
.\.pio.ps1 run -e esp32_ble
.\.pio.ps1 run -e esp32_wifi_ble
```

Run the browser client:

```bash
npm run web -w @teslacanmodder/client
```

Open the Expo URL in Chrome or Edge for Web Serial flashing and runtime control.

## Firmware Targets

Supported release environments:

- `esp32`: USB serial only
- `esp32_wifi`: USB serial + WiFi REST API/dashboard
- `esp32_ble`: USB serial + BLE
- `esp32_wifi_ble`: USB serial + WiFi + BLE

Bus lanes are controlled with build flags:

| Bus     | X179 Pins | Build Flag           | Default |
| ------- | --------- | -------------------- | ------- |
| Chassis | 13-14     | `BUS_CHASSIS_ACTIVE` | On      |
| Vehicle | 9-10      | `BUS_VEHICLE_ACTIVE` | Off     |
| Body    | 2-3       | `BUS_BODY_ACTIVE`    | Off     |

Example with all three lanes enabled:

```powershell
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_CHASSIS_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\.pio.ps1 run -e esp32_wifi_ble
```

Tagged releases publish merged flash-ready ESP32 images through GitHub Actions. The client flasher consumes those release assets directly.

## Common Validation

```bash
npm run test:all
npm run lint:all
npm run typecheck:client
```

Firmware-only validation:

```powershell
cd firmware
.\.pio.ps1 test -e native
```

## Documentation

Start here:

- `docs/README.md`
- `docs/guides/getting-started.md`
- `docs/guides/full-setup.md`
- `docs/checklists/release-checklist.md`
- `docs/architecture/unified-client-guide.md`

The client docs screen and the repo docs folder are the same source of truth.

## Legacy Research

`legacy/` contains archived upstream and community repositories used for comparison, reverse-engineering notes, and feature archaeology. Shipping code lives outside that tree.

See:

- `docs/legacy/README.md`
- `docs/legacy/COMPARISON.md`
- `THIRD_PARTY_LICENSES`

## License

GPL-3.0

# Display (main center screen, 0-127)

maindisplay:0-127

# Power (vehicle state control)

power:acc:on, power:acc:off, power:off, power:ready

```

### Advanced Features

```

# Window Vent (0x119)

window:vent:N # N = 0..100
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

- ✅ Desktop Chrome/Edge - Web Serial flashing + runtime control
- ✅ Android Chrome - Runtime control via Bluetooth
- ⚠️ Other browsers - Guide mode only

## Workspace

This is an npm workspace monorepo:

```

packages/protocol — shared types, commands, decoder, parser (@teslacanmodder/protocol)
firmware — PlatformIO ESP32/Arduino firmware
client — React Native + Expo client app for browser, iOS, and Android
tools — CLI debug utilities

````

```bash
npm install          # install all workspaces
npm run test:all     # run all tests (protocol + client + tools + firmware)
````

## Testing

| Layer    | Runner                    | Tests |
| -------- | ------------------------- | ----- |
| Firmware | PlatformIO Unity          | 178   |
| Protocol | Jest (ESM)                | 102   |
| Client   | Jest + Testing Library/RN | 138   |

## CI

GitHub Actions runs on push to `main` and all PRs:

- **firmware** — PlatformIO native tests
- **protocol** — shared package tests
- **client** — Jest tests
- **tools** — CLI tests
- **docker** — docker compose build

## Documentation

- `docs/unified-setup-guide.md` - Canonical end-to-end setup (build, flash, connect, validate)
- `docs/unified-client-guide.md` - Canonical client architecture, setup, monitor workflows, and migration notes
- `docs/reference/can-ids.md` - Master CAN ID reference table
- `firmware/README.md` - Firmware details
- `docs/WIFI_BOARD_GUIDE.md` - ESP32 WiFi dashboard setup

## Legacy References

The `legacy/` directory contains **83 external repositories** added as read-only
git submodules for research and comparison. No code is copied into the main
codebase.

| Category                     | Count | Examples                                    |
| ---------------------------- | ----- | ------------------------------------------- |
| FSD CAN Mod                  | 14    | jvanakker, juamiso, herrfrei, JelloEa       |
| CAN Monitoring / Analysis    | 12    | ekr-candash, hanswolff, bruvv, mikegapinski |
| CAN Database / Decoding      | 8     | joshwardell-model3dbc, krconv, talas9       |
| Flipper Zero                 | 3     | hypery11, J0811, canhackers-jupiter         |
| BLE / Bluetooth              | 2     | wimaha-TeslaBleHttpProxy, DemiVis           |
| Steering / EPAS              | 3     | gregjhogan, sydneyg007 (×2)                 |
| Battery / Charging           | 3     | jomytec-My_TeslaBMS, jamiejones85, oliwiah  |
| Other (logging, apps, tools) | 38    | rossklonowski-CANserver, tesberry, uhi22    |

- **Individual analyses** → [`docs/legacy/<repo>.md`](docs/legacy/)
- **Synthesis report** → [`docs/legacy-summary.md`](docs/legacy-summary.md)
- **License compliance** → [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md)

## License

GPL-3.0
