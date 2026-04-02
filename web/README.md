# TeslaCANModder Web Client

React + Vite frontend for TeslaCANModder.

## Architecture

- `src/lib/board/*`: canonical board commands, board protocol parsing, and the serial board client class
- `src/hooks/useBoardLink.js`: canonical board-management hook for USB and paired HC-05 serial ports
- `src/lib/can/core/*`: compatibility helpers that now delegate the board protocol pieces to `src/lib/board/*`
- `src/packages/*`: feature packs layered on top of the base CAN viewer
- `src/packages/fsd/*`: FSD-specific state adapter, command helpers, and dashboard panel
- `public/reference/*`: real connector reference images used by the setup guide

## Commands

```bash
npm install
npm run dev
npm run build
npm run lint
```

## Docker

Run the web client in Docker dev mode:

```bash
docker compose up web
```

The Vite server listens on `http://localhost:4173`.
Heartbeat path: `http://localhost:4173/heartbeat.json`
The dev heartbeat is provided by Vite middleware, not a static file.

Run the production web image:

```bash
docker compose up web-prod
```

That serves the built app on `http://localhost:8080`.
Heartbeat path: `http://localhost:8080/heartbeat.json`
The production heartbeat is provided by the Nginx config in `web/nginx.conf`.

## What the UI expects

- A board speaking the JSON protocol implemented in [bridge.h](C:/Users/boran/Masaüstü/Tesla-CAN-Mod/hardware/lib/board/bridge.h)
- Boot/status metadata with `hw`, `can`, `drv`, and `variant`
- Status messages with `fsd`, `sp`, `up`, transport capability, Bluetooth enabled state, and install readiness
- Frame messages with `dir`, `id`, `dlc`, and hex payload `d`

## Ordered Kit Notes

- The setup guide is tuned for the ordered Arduino Uno R3 CH340 clone, MCP2515 + TJA1050 board, and optional HC-05 module.
- USB is the required first connection path for the first flash and first diagnostics. After the HC-05 is paired in the operating system, the same Web Serial flow can open its paired COM port.
- The dashboard is now capability-aware: desktop Chrome/Edge stays the primary flash and recovery path, Android Chrome is the target phone runtime path, and unsupported phone browsers fall back to guide-first UX instead of broken serial controls.
- The project-specific HC-05 mapping is `HC-05 TX -> Arduino D4` and `Arduino D5 -> divider -> HC-05 RX`; this differs from many public examples that use `D0/D1` or other SoftwareSerial pins.
- The ordered HC-05 should be wired as a `3.3V` device and the `D5 -> RX` line must stay behind a resistor divider.
- The ordered MCP2515 board uses an `8 MHz` crystal, and the AVR firmware defaults are compiled for that clock.
- The setup guide now uses the local X179 references that mark `pin 1` as `+12V`, `pin 20` as `GND`, `pin 13` as `CAN-H`, and `pin 14` as `CAN-L`.
- The guide is bench-first: flash and validate the board over USB power, then move permanent board power to X179 `pin 1` and `pin 20` through the converter input and feed the Uno from the converter USB output.
- Jumper wires are treated as short internal board links, not as the final vehicle-side X179 or converter-input harness.
- The setup guide now renders a Mermaid-based HC-05 wiring schema and also exposes the raw Mermaid source for reuse in docs or issues.

## Runtime Command Surface

- Base viewer commands: `ping`, `status`, `stream:on`, `stream:off`, `variant:<hw4|hw3|legacy>`
- FSD package commands: `fsd:on`, `fsd:off`, `profile:<0-4>`

## Direct project flashing

The `/firmware-builds/*` route is only available in local development. It is served by the custom Vite middleware in [vite.config.js](C:/Users/boran/Masaüstü/Tesla-CAN-Mod/web/vite.config.js) and reads `.hex` files from `../hardware/.pio/build`. The direct project button now targets the shared `uno` firmware image, and vehicle selection happens at runtime in the dashboard.

For the first flash, build the firmware from `hardware` first:

```bash
cd hardware
pio run -e uno
```

The expected first-flash file is `hardware/.pio/build/uno/firmware.hex`, whether you fetch it through the dev-only direct flash button or load it manually through the custom file picker.

To generate that same file in Docker:

```bash
docker compose run --rm firmware
```

That writes `hardware/.pio/build/uno/firmware.hex` directly into the host project tree through the bind-mounted workspace.
