# TeslaCANModder

A browser-based dashboard, frame inspector, setup guide, and firmware flasher for Tesla CAN mod hardware.

## Features

- Modular web client with a core CAN viewer plus feature packages layered on top.
- Board control path designed for one web client protocol over Arduino USB or a paired HC-05 serial COM port.
- USB is the mandatory first-flash and recovery path; HC-05 is an optional runtime link after the wired install is already stable.
- One Uno firmware image that includes `HW4`, `HW3`, and `Legacy` vehicle handlers and switches between them at runtime.
- Live dashboard for board status, uptime, message rate, and stream health.
- Separate FSD package for viewing and overriding FSD state and speed profile.
- Real-time CAN frame stream viewer with recent payload inspection.
- Wiring and safety checklist for the board, CAN interface, and vehicle CAN bus.
- WebSerial-based STK500 flasher for custom `.hex` files.
- Optional direct-project flashing during local development when the Vite dev server can serve PlatformIO build artifacts.

## Hardware

The active firmware and web client now cover:

- Arduino Uno (or compatible) + MCP2515 + optional HC-05

The current defaults are tuned for the ordered SAMM kit:

- Arduino Uno R3 SMD CH340 clone
- MCP2515 CAN board with TJA1050 transceiver and 8 MHz crystal
- Optional HC-05 Bluetooth serial module

Important notes:

- Disable the MCP2515 termination resistor before connecting to the vehicle bus.
- This firmware does not use the Uno hardware UART pins for HC-05. It maps Bluetooth to `D4`/`D5` through `SoftwareSerial`, so the guide differs from many public HC-05 examples.
- The HC-05 RX pin must be level-shifted from Arduino `D5`.
- Treat the ordered HC-05 as a `3.3V` module. Its product listing specifies `3.3V` operating voltage and `50 mA` current draw, so a dedicated `3.3V` regulator is safer than assuming raw `5V` tolerance.
- HC-05 support is optional. AVR builds default to `BOARD_ENABLE_BT=1`; set `-DBOARD_ENABLE_BT=0` for a USB-only firmware image.
- Uno environments explicitly target the ordered `8 MHz` MCP2515 oscillator through `BOARD_CAN_CLOCK_MHZ=8`.
- The CH340 USB bridge on the ordered Uno clone may require a driver on Windows 7 or Mac OS X. Linux and Windows 8+ are typically plug-and-play.
- Model 3/Y installs should use the X179 connector when present. The setup guide now includes the bundled X179 pinout images.
- The local X179 references used by the setup guide mark `pin 1` as `+12V`, `pin 20` as `GND`, `pin 13` as `CAN-H`, and `pin 14` as `CAN-L`.
- The install flow is USB first, X179 second: flash and validate the Uno on the bench over USB, then move permanent board power to X179 `pin 1` and `pin 20` through the converter input and feed the Uno from the converter USB output. Do not feed X179 `+12V` directly into the Uno `5V` pin.
- The purchased converter class is a `9V-36V -> 5V / 3A` dual-USB step-down board. Treat its input side as the permanent vehicle-power connection and its USB output as the permanent Uno power path.
- Jumper wires are acceptable for short board-to-board links during bench work or inside an enclosure, but not for the final X179 or converter-input harness in a vehicle.
- The web setup guide now includes a rendered Mermaid HC-05 wiring schema plus the raw Mermaid source so the same diagram can be reused in markdown documentation.
- Use the in-app Setup Guide for the current pinout and install checklist.

## Development

```bash
cd web
npm install
npm run dev
```

Production build:

```bash
cd web
npm run build
```

## Docker

Container paths are now available for both the firmware build and the web UI.

Build the Uno firmware HEX in Docker:

```bash
docker compose run --rm firmware
```

That writes the same first-flash artifact into the real host project folder:

```text
hardware/.pio/build/uno/firmware.hex
```

Run the web app in Docker dev mode:

```bash
docker compose run --rm firmware
docker compose up web
```

The app is then available at:

```text
http://localhost:4173
```

Dev heartbeat:

```text
http://localhost:4173/heartbeat.json
```

The dev heartbeat is served by the Vite middleware and returns live JSON with `mode: "dev"`.

If you want a static production web container instead of the Vite dev server:

```bash
docker compose up web-prod
```

That serves the built frontend at `http://localhost:8080`.

Production heartbeat:

```text
http://localhost:8080/heartbeat.json
```

The production heartbeat is served by Nginx and returns `mode: "production"`.

## Firmware

PlatformIO project files live in [hardware/platformio.ini](C:/Users/boran/Masaüstü/Tesla-CAN-Mod/hardware/platformio.ini). The active environment is:

- `uno`

That single Uno image contains all supported vehicle handlers. Select the active vehicle at runtime from the web dashboard or with `variant:hw4`, `variant:hw3`, or `variant:legacy`.

First-time flash path:

```bash
cd hardware
pio run -e uno
```

The canonical first image is `hardware/.pio/build/uno/firmware.hex`. The web flasher and setup guide now treat that `hardware` build output as the default first-flash source.

If the global `C:\Users\<user>\.platformio` state is broken on Windows, use the repo-local wrapper instead:

```powershell
cd hardware
powershell -ExecutionPolicy Bypass -File .\pio.ps1 run -e uno
```

That wrapper forces PlatformIO to use `hardware/.pio-home` instead of the global PlatformIO home.

## Firmware Architecture

The hardware code now follows the same core-plus-packages shape as the web client:

- Canonical board control classes live in `hardware/lib/board/*`
- Canonical CAN data and helpers live in `hardware/lib/can/*`
- Canonical driver classes live in `hardware/lib/drivers/runtime/*` and `hardware/lib/drivers/mcp2515/*`
- `board/app.h`, `board/bridge.h`, `board/transport.h`, `board/commands.h`, and `board/state.h` are the main firmware control surface for the web client
- Canonical handler classes live in `hardware/lib/handlers/runtime/*` and `hardware/lib/handlers/variants/*`
  - `handler.h` defines the generic board-facing handler contract
  - `stack.h` provides the reusable package-dispatch runtime
  - typed handler state now lives in `state.h`
- Feature packages live in `hardware/lib/packages/*`
  - Runtime contracts in `hardware/lib/packages/runtime/*`, with canonical files centered on `context.h`, `package.h`, and `dispatch.h`
  - Aggregators in `hardware/lib/packages/index.h` and `hardware/lib/packages/fsd/index.h`
  - Variant folders in `hardware/lib/packages/fsd/legacy`, `hardware/lib/packages/fsd/hw3`, and `hardware/lib/packages/fsd/hw4`
  - Canonical package entry files now use short names such as `follow_distance.h`, `fsd_mux0.h`, `nag.h`, `profile.h`, and `stalk_speed.h`
- Native tests use local helpers in `hardware/test/support/*` instead of shipping mock drivers in the production include tree
- C++ modules now live directly under `hardware/lib/*` and use class-based entry points instead of large flat headers

## Web Client Architecture

- Canonical board client code lives in `web/src/lib/board/*`
- `web/src/hooks/useBoardLink.js` is the main hook used by the UI
- `web/src/hooks/useWebSerial.js` remains as a compatibility export
- The same line-based JSON protocol is used whether the browser connects to the Arduino over CH340 USB or to a paired HC-05 serial port
- Boot and status metadata now include transport capability and install readiness so the web UI can distinguish `bench-ready`, `installed-power-ready`, and `runtime-ready` states without a second protocol

## Runtime Commands

The board accepts a small serial command set over USB and, when enabled, over HC-05:

- `ping`
- `status`
- `stream:on`
- `stream:off`
- `fsd:on`
- `fsd:off`
- `profile:<0-4>`
- `variant:<hw4|hw3|legacy>`
