# Tesla CAN Modder

[![CI](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml/badge.svg)](https://github.com/babico/TeslaCANModder/actions/workflows/ci.yml)

Open-source Tesla CAN bus firmware, cross-platform client, shared protocol library, and diagnostics tooling built around the ESP32-S DevKit and the Tesla X179 diagnostic connector.

## Features

- **Multi-bus CAN architecture** — up to 3 MCP2515 buses (Chassis, Vehicle, Body) on the X179 connector
- **48 vehicle feature modules** — window vent, sentry, climate, charge, DAS Drive, gamepad injection, BMS, TPMS, and more
- **Cross-platform client** — Expo app for Web, iOS, and Android with Web Serial flashing, live CAN monitor, and in-app docs
- **DAS Drive** — openpilot-style gamepad CAN injection with safety envelope, rate limiting, and NVS persistence
- **BLE Gamepad** — NimBLE central scanner/pairing, 16-button bindings, 6 analog axes with deadzone/expo tuning
- **Tesla BLE Key** — P-256 ECDSA key pair generation, role-based access (owner/charging_manager), NFC card pairing, authenticated commands (wake, charge, climate)
- **WiFi REST API + Dashboard** — on-device HTTP API and HTML dashboard served over soft-AP
- **Browser flasher** — flash firmware directly from Chrome/Edge using Web Serial (no toolchain needed)
- **CAN frame decoder** — 577 Tesla frames decoded from the mikegapinski dataset
- **Shared protocol package** — `@teslacanmodder/protocol` with commands, types, decoder, and parser
- **Debug CLI** — `tcm-debug` serial tool with 12 command modules for bench work and validation

## Repository Structure

```text
TeslaCANModder/
├── firmware/                      PlatformIO ESP32 firmware (C++)
│   ├── lib/core/                  shared types, driver, persistence, log, CAN plumbing
│   ├── lib/io/                    transports: serial, WiFi, BLE
│   ├── lib/client/                REST API, dashboard, command dispatch, gamepad
│   ├── lib/vehicle/can/           Tesla CAN logic (48 feature modules + variant handlers)
│   ├── lib/vehicle/ble/           Tesla BLE protocol
│   ├── src/esp32/                 firmware entry point
│   └── test/                      PlatformIO Unity native test suites
├── client/                        Expo app (Web, iOS, Android)
│   ├── src/                       components, screens, state, hardware, design system
│   └── tests/                     Jest + React Native Testing Library
├── packages/protocol/             @teslacanmodder/protocol (shared TS package)
├── tools/                         tcm-debug CLI + serial-to-HTTP bridge
├── docs/                          canonical markdown docs (rendered in-app)
├── legacy/                        80+ external reference submodules (read-only)
└── scripts/                       workspace-level validation and smoke scripts
```

## Quick Start

### Prerequisites

- Node.js >= 18
- Python 3.11+ with PlatformIO CLI (`pip install platformio`) for firmware work

### Install

```bash
git clone https://github.com/babico/TeslaCANModder.git
cd TeslaCANModder
npm install
```

### Build Firmware

```powershell
cd firmware
.\pio.ps1 run -e esp32_chassis_8mhz
```

### Run the Client (Web)

```bash
npm run web -w @teslacanmodder/client
```

Open the Expo URL in Chrome or Edge for Web Serial flashing and runtime control.

### Run Tests

```bash
npm run test:all       # all workspaces + firmware native tests
npm run lint:all       # ESLint + Prettier check
```

## Firmware

### Environments

| Environment                   | Features                      |
| ----------------------------- | ----------------------------- |
| `esp32_chassis_8mhz`          | Serial + Chassis CAN          |
| `esp32_wifi_chassis_8mhz`     | Serial + WiFi + Chassis       |
| `esp32_ble_chassis_8mhz`      | Serial + BLE + Chassis        |
| `esp32_wifi_ble_chassis_8mhz` | Serial + WiFi + BLE + Chassis |
| `native`                      | Host-only tests               |

Environments follow the naming convention: `esp32[_wifi][_ble][_chassis][_vehicle][_body][_8mhz|_16mhz]`

### Bus Lanes (X179 Connector)

| Bus     | Index | X179 Pins | Build Flag           |
| ------- | ----- | --------- | -------------------- |
| Chassis | 0     | 13-14     | `BUS_CHASSIS_ACTIVE` |
| Vehicle | 1     | 9-10      | `BUS_VEHICLE_ACTIVE` |
| Body    | 2     | 2-3       | `BUS_BODY_ACTIVE`    |

All buses are opt-in. Enable multiple lanes:

```powershell
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_CHASSIS_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\pio.ps1 run -e esp32_wifi_ble_chassis_8mhz
```

### Vehicle Features (48 modules)

Air Recirculation, Auto Lane Change, Ban Detect/Shield, BMS, Charge, Climate, DAS Drive, Display, Drive Context, Drive Mode, FSD, ISA Chime, Lights, Lock, Mirror, Motor Temps, MQTT Bridge, Nag, Pedal, Power, Powertrain, Precondition, Profile, Regen, Region, Seat, Seatbelt, Sentry, Stop Mode, Stream, Summon, TLSSC, TPMS, Track Mode, Trunk, Turn Signal, Variant, Vehicle Config, Wheel Speeds, Window, Wiper, Tesla BLE Key, and more.

## Client App

Cross-platform Expo application with six main screens:

| Screen    | Purpose                                         |
| --------- | ----------------------------------------------- |
| Dashboard | Live vehicle state overview                     |
| Controls  | Feature toggles, DAS Drive panel, Gamepad panel |
| Drive     | DAS Drive real-time HUD                         |
| Console   | Serial/WiFi command terminal                    |
| Flasher   | Browser-based ESP32 firmware flasher            |
| Docs      | In-app documentation (renders from `docs/`)     |

### Browser Support

- ✅ Desktop Chrome/Edge — Web Serial flashing + runtime control
- ✅ Android Chrome — Runtime control via Bluetooth
- ⚠️ Other browsers — Guide mode only

## Commands

### Basic Controls

```text
power:acc:on, power:acc:off, power:off, power:ready
maindisplay:0-127
window:vent:open, window:vent:close
sentry:on, sentry:off
climate:keep, climate:off
charge:start, charge:stop
```

### Drive Configuration

```text
pedal:standard, pedal:chill, pedal:sport
regen:off, regen:low, regen:std, regen:max
stop:creep, stop:roll, stop:hold
```

### DAS Drive (Gamepad CAN Injection)

```text
das:arm, das:disarm, das:status
gamepad:scan, gamepad:pair, gamepad:unpair
gamepad:bind:<n>:<cmd>, gamepad:axis:<n>:<dz|expo|inv>:<v>
```

### Tesla BLE Key & Vehicle Control

```text
tesla:key:gen, tesla:key:show, tesla:key:role:<owner|charging_manager>
tesla:key:send, tesla:vin:<VIN>
tesla:wake
tesla:charge:start, tesla:charge:stop, tesla:charge:amps:<1-32>, tesla:charge:limit:<50-100>
tesla:climate:on, tesla:climate:off
```

See `docs/reference/commands.md` for the full command reference.

## Validation Commands

| Task                     | Command                            |
| ------------------------ | ---------------------------------- |
| Run all tests            | `npm run test:all`                 |
| Lint + format check      | `npm run lint:all`                 |
| Typecheck protocol       | `npm run typecheck:protocol`       |
| Typecheck client         | `npm run typecheck:client`         |
| Validate serial contract | `npm run validate:serial-contract` |
| Firmware native tests    | `npm run test:firmware`            |
| Protocol tests           | `npm run test:protocol`            |
| Client tests             | `npm run test:client`              |
| Tools tests              | `npm run test:tools`               |

## Testing

| Layer    | Runner                    | Location             |
| -------- | ------------------------- | -------------------- |
| Firmware | PlatformIO Unity (native) | `firmware/test/`     |
| Protocol | Jest (ESM)                | `packages/protocol/` |
| Client   | Jest + Testing Library/RN | `client/tests/`      |
| Tools    | Jest (ESM)                | `tools/test/`        |

## CI/CD

GitHub Actions runs on push to `main` and all PRs with 10 jobs:

- **firmware** — PlatformIO native tests + size regression check
- **protocol** — shared package tests
- **e2e-smoke** — end-to-end smoke tests
- **client** — Jest + Testing Library tests
- **tools** — CLI tests
- **docker** — docker compose build
- **docker-smoke** — container health check
- **lint** — ESLint + Prettier
- **markdown-lint** — markdownlint-cli2
- **security-audit** — dependency audit + license compatibility

Tagged releases publish flash-ready ESP32 images that the client flasher consumes directly.

## Documentation

| Topic               | Path                                        |
| ------------------- | ------------------------------------------- |
| Getting started     | `docs/guides/getting-started.md`            |
| Full setup          | `docs/guides/full-setup.md`                 |
| Hardware setup      | `docs/guides/hardware-setup.md`             |
| Flasher quickstart  | `docs/guides/flasher-quickstart.md`         |
| DAS Drive guide     | `docs/guides/das-drive.md`                  |
| Security            | `docs/guides/security.md`                   |
| Command reference   | `docs/reference/commands.md`                |
| CAN IDs             | `docs/reference/can-ids.md`                 |
| CAN protocol        | `docs/reference/can-protocol.md`            |
| Signal matrix       | `docs/reference/signal-matrix.md`           |
| WiFi API            | `docs/reference/wifi-api.md`                |
| Vehicle features    | `docs/reference/vehicle-features.md`        |
| Client architecture | `docs/architecture/unified-client-guide.md` |
| Release checklist   | `docs/checklists/release-checklist.md`      |
| Debug guide         | `docs/troubleshooting/debug-guide.md`       |

## Legacy Research

The `legacy/` directory contains 80+ external repositories as read-only git submodules for reverse-engineering research and feature archaeology. No code is copied into the shipping codebase.

- Individual analyses: `docs/legacy/<repo>.md`
- Comparison report: `docs/legacy/COMPARISON.md`

## Contributing

See `CONTRIBUTING.md` for setup instructions, coding standards, and PR guidelines.

## License

[WTFPL v2](LICENSE)
