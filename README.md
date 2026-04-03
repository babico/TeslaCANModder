# TeslaCANModder

TeslaCANModder is a browser-based control and monitoring stack for an Arduino Uno CAN gateway used with Tesla CAN modification workflows. The repo includes the Uno firmware, the web UI, setup guidance, a first-flash path, and legacy references kept for comparison.

## What This Repo Contains

- `hardware/`: PlatformIO firmware project for the Arduino Uno build
- `hardware/lib/`: firmware modules split into `board`, `can`, `drivers`, `handlers`, and `packages`
- `web/`: React + Vite dashboard, setup guide, and browser flasher
- `legacy/`: external reference projects kept as Git submodules

## Product Model

The active project follows one operating model everywhere:

1. USB is required for the first flash, first diagnostics, and recovery.
2. X179 power through the purchased converter is the permanent installed power path.
3. HC-05 is optional and is only introduced after the wired system is already stable.
4. One Uno firmware image contains `hw4`, `hw3`, and `legacy` handlers and switches variant at runtime.

The web UI is transport-aware, but the board protocol stays the same over USB and HC-05.

## Canonical Hardware Bundle

The project is currently tuned for this ordered hardware:

- Arduino Uno R3 CH340 clone
- MCP2515 CAN board with TJA1050 transceiver and `8 MHz` crystal
- HC-05 Bluetooth serial module
- `9V-36V -> 5V / 3A` USB-output converter
- Tesla X179 connector for installed power and CAN access

## Wiring Contract

These rules are the canonical wiring assumptions used by the firmware, guide, and UI copy.

### HC-05

- `HC-05 TXD -> Arduino D4`
- `Arduino D5 -> 1k resistor -> divider node -> HC-05 RXD`
- `divider node -> 2k resistor -> GND`
- `HC-05 VCC -> dedicated 3.3V regulator output`
- `HC-05 GND -> common ground`

### MCP2515

- `MCP2515 VCC -> Arduino 5V`
- `MCP2515 GND -> Arduino GND`
- `MCP2515 CS/SCK/MISO/MOSI/INT -> current Uno firmware pin map`
- disable the MCP2515 termination resistor before connecting to a live vehicle bus

### X179 and installed power

- `X179 pin 1 -> converter VIN+`
- `X179 pin 20 -> converter VIN-`
- `converter USB output -> Uno USB port`
- `X179 pin 13 -> MCP2515 CAN-H`
- `X179 pin 14 -> MCP2515 CAN-L`

## Things The Project Explicitly Forbids

- direct `Uno TX -> HC-05 RX` without a divider
- powering the ordered HC-05 as if it were a raw `5V` module
- feeding X179 `12V` directly into the Uno `5V` pin
- first flashing on vehicle-installed power
- using loose jumper wires as the final X179 or converter-input harness
- leaving CAN-module termination enabled on the live vehicle bus

## Install Workflow

### Stage A: bench flash over USB

- connect the Uno to a PC with USB only
- build `hardware/.pio/build/uno/firmware.hex`
- flash the shared `uno` image
- open the dashboard over USB
- verify `ping`, `status`, `variant:*`, and `stream:on` / `stream:off`

### Stage B: move to X179 power

- wire X179 `pin 1` and `pin 20` to the converter input
- verify converter output before plugging the Uno in
- power the Uno from the converter USB output
- verify the board still boots and answers commands

### Stage C: connect CAN

- connect X179 `pin 13` and `pin 14` to MCP2515 `CAN-H` / `CAN-L`
- confirm traffic appears in the dashboard
- confirm the selected runtime variant matches the target car behavior

### Stage D: optional HC-05

- wire the regulator and divider
- pair the HC-05 in the host OS
- connect through the same browser UI using the paired serial port
- verify `ping`, `status`, and one control action

## Repo Architecture

### Firmware

Firmware code is split by concern under `hardware/lib/`:

- `board/`: protocol bridge, state, transport metadata, and app bootstrap
- `can/`: CAN frame types and helpers
- `drivers/`: hardware driver contracts and MCP2515 implementation
- `handlers/`: runtime handler interface and per-variant handler classes
- `packages/`: reusable feature packages layered into the handlers

The board-facing control surface is centered on:

- `hardware/lib/board/app.h`
- `hardware/lib/board/bridge.h`
- `hardware/lib/board/commands.h`
- `hardware/lib/board/state.h`
- `hardware/lib/board/transport.h`

### Web

The web app is split similarly:

- `web/src/lib/board/*`: command constants, protocol normalization, browser capability detection, serial client
- `web/src/hooks/useBoardLink.js`: main board hook used by the UI
- `web/src/components/*`: dashboard, guide, flasher, explorer
- `web/src/packages/*`: feature packages layered on top of the base viewer
- `web/src/styles/*`: CSS split by app area instead of one giant file

## Runtime Command Surface

The board accepts the same commands over USB and HC-05:

- `ping`
- `status`
- `stream:on`
- `stream:off`
- `variant:hw4`
- `variant:hw3`
- `variant:legacy`
- `fsd:on`
- `fsd:off`
- `profile:<0-4>`

## Development

### Web

```bash
cd web
npm install
npm run dev
```

Useful checks:

```bash
cd web
npm run lint
npm run build
```

### Firmware

Preferred local wrapper:

```powershell
cd hardware
.\pio.ps1 run -e uno
```

That produces:

```text
hardware/.pio/build/uno/firmware.hex
```

Run native firmware tests:

```powershell
cd hardware
.\pio.ps1 test -e native
```

## Docker

Build the Uno firmware hex with Docker:

```bash
docker compose run --rm firmware
```

Run the web app in dev mode:

```bash
docker compose up web
```

Run the production web container:

```bash
docker compose up web-prod
```

Heartbeat endpoints:

- dev: `http://localhost:4173/heartbeat.json`
- prod: `http://localhost:8080/heartbeat.json`

## Browser Support Model

- Desktop Chrome or Edge: primary path for flashing, service, and recovery
- Android Chrome: target mobile runtime-control path
- other mobile browsers: guide-first fallback, not guaranteed full serial control

The UI detects `navigator.serial` and degrades to a compatibility-first experience when live serial control is not available.

## Legacy References

The `legacy/` directory contains external reference projects as Git submodules. They are not the active codebase, but they are useful when comparing behavior or checking historical implementation details.

## Additional Docs

- firmware details: `hardware/README.md`
- web app details: `web/README.md`
