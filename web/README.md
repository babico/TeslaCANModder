# TeslaCANModder Web

This app is the browser UI for TeslaCANModder. It handles setup guidance, serial connection, runtime board control, CAN monitoring, package-specific controls, and first-flash UX.

## Main Responsibilities

- detect whether the current browser can use Web Serial
- connect to the board over USB or a paired HC-05 serial port
- normalize boot, status, log, ack, error, and frame messages
- show runtime vehicle and transport state
- expose package-specific controls such as FSD/profile actions
- guide the user through the USB-first and X179-second installation flow

## App Structure

### Core runtime

- `src/lib/board/commands.js`
  board command constants and helpers
- `src/lib/board/protocol.js`
  message normalization, telemetry shaping, and list trimming helpers
- `src/lib/board/client.js`
  serial client wrapper around the browser connection flow
- `src/lib/board/capabilities.js`
  browser and device capability detection used by the UI
- `src/hooks/useBoardLink.js`
  main stateful hook that the UI uses to connect, send commands, and consume board messages

### UI

- `src/components/Dashboard.jsx`
  main runtime control and monitor surface
- `src/components/SetupGuide.jsx`
  installation guide, Mermaid diagrams, and staged validation workflow
- `src/components/Flasher.jsx`
  first-flash and firmware-selection UX
- `src/components/CanExplorer.jsx`
  frame inspection surface

### Packages

- `src/packages/*`
  package registry and package-specific UI
- `src/packages/fsd/*`
  FSD package adapter and panel

### Styling

Styles are split under `src/styles/` by app area:

- `base.css`
- `app-shell.css`
- `dashboard.css`
- `setup-guide.css`
- `explorer.css`
- `shared-pages.css`
- `responsive.css`

`src/index.css` only imports those files.

## Board Protocol Expectations

The UI expects the firmware to use the line-based JSON protocol implemented by `hardware/lib/board/bridge.h`.

Important message types:

- `boot`
- `status`
- `frame`
- `log`
- `ack`
- `error`
- `pong`

Important status metadata:

- `hw`
- `can`
- `drv`
- `variant`
- `bt`
- `cap`
- `ready`
- `fsd`
- `sp`
- `up`

The UI uses that metadata to distinguish:

- board identity
- transport capability
- current runtime variant
- whether Bluetooth is compiled in
- whether the install is still bench-only or already seeing live CAN traffic

## Dashboard Model

The dashboard is built around one board link and two usage modes:

- desktop: full tile-based layout with control, monitoring, and advanced console
- mobile: simpler one-column flow split into setup, control, monitor, and advanced sections

The tile layout is user-adjustable. Tile order, visibility, and size are saved in local browser storage so the dashboard reopens in the same shape.

## Setup Guide Model

The guide is intentionally staged:

1. ordered hardware summary
2. preflight and safety
3. bench flash over USB
4. dashboard validation on the bench
5. move power to X179 through the converter
6. connect CAN
7. optional HC-05 wiring and pairing
8. first vehicle validation
9. troubleshooting by symptom

Mermaid diagrams are used for the wiring and process views so the diagrams stay versionable in the repo.

## Development

Install and run the app locally:

```bash
npm install
npm run dev
```

Checks:

```bash
npm run lint
npm run build
```

## Docker

Start the dev server through Docker:

```bash
docker compose up web
```

That serves the app at:

```text
http://localhost:4173
```

Dev heartbeat:

```text
http://localhost:4173/heartbeat.json
```

Run the production container:

```bash
docker compose up web-prod
```

That serves the built app at:

```text
http://localhost:8080
```

Production heartbeat:

```text
http://localhost:8080/heartbeat.json
```

## First Flash

The first flash still depends on the firmware build from `hardware`:

```powershell
cd hardware
.\pio.ps1 run -e uno
```

The canonical file is:

```text
hardware/.pio/build/uno/firmware.hex
```

During local development, the Vite middleware can also expose that built hex directly for the flasher UI.

## Supported Usage Model

- Desktop Chrome or Edge:
  full flashing, control, and recovery path
- Android Chrome:
  target phone runtime-control path
- unsupported browsers:
  guide and compatibility fallback, not guaranteed live serial control

## Ordered Hardware Assumptions

- Uno CH340 clone
- MCP2515 + TJA1050 board with `8 MHz` crystal
- HC-05 wired to `D4` and `D5` with a divider on `D5 -> RX`
- X179-powered install using the purchased converter

The guide and dashboard intentionally treat USB as the first bring-up path and HC-05 as a later optional transport.
