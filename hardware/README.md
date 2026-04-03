# TeslaCANModder Firmware

This directory contains the PlatformIO firmware project for the Arduino Uno build used by TeslaCANModder.

## Build Targets

The active firmware target is:

- `uno`

That single image contains all supported vehicle handlers and switches the active variant at runtime. The board does not need separate `hw3`, `hw4`, or `legacy` firmware builds anymore.

## Build And Test

Build the firmware:

```powershell
.\pio.ps1 run -e uno
```

Run native tests:

```powershell
.\pio.ps1 test -e native
```

The canonical first-flash artifact is:

```text
.pio/build/uno/firmware.hex
```

## Why `pio.ps1` Exists

`pio.ps1` wraps PlatformIO so the project can use a repo-local `.pio-home` instead of depending on a healthy global PlatformIO install. On Windows it also works around Unicode path issues by using an ASCII drive mapping when needed.

## Directory Layout

- `src/main.cpp`
  Arduino entry point
- `lib/board/*`
  board runtime, protocol bridge, state, commands, and transport metadata
- `lib/can/*`
  CAN frame types and helpers
- `lib/drivers/*`
  driver contracts and hardware driver implementations
- `lib/handlers/*`
  generic handler interface and runtime variant handlers
- `lib/packages/*`
  reusable feature packages used by the handlers
- `test/*`
  native host-side tests

## Firmware Runtime Model

### Board layer

The board layer owns:

- boot and ready messages
- serial command parsing
- JSON message emission
- transport metadata
- board state such as variant, stream state, install readiness, and Bluetooth capability

Important files:

- `lib/board/app.h`
- `lib/board/bridge.h`
- `lib/board/commands.h`
- `lib/board/state.h`
- `lib/board/transport.h`

### Driver layer

The driver layer hides the physical CAN controller details from the rest of the firmware.

Important files:

- `lib/drivers/runtime/driver.h`
- `lib/drivers/mcp2515/arduino_mcp2515.h`

### Handler layer

Handlers map incoming CAN traffic to variant-specific behavior.

Important files:

- `lib/handlers/runtime/handler.h`
- `lib/handlers/runtime/stack.h`
- `lib/handlers/runtime/state.h`
- `lib/handlers/variants/hw3.h`
- `lib/handlers/variants/hw4.h`
- `lib/handlers/variants/legacy.h`

### Package layer

Packages hold smaller, reusable slices of behavior used by the variant handlers. The FSD-related packages are grouped by variant under `lib/packages/fsd/`.

## Serial Protocol

The firmware uses one line-based JSON protocol over both USB and HC-05. The transport changes, but the message format does not.

Common inbound commands:

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

Common outbound message types:

- `boot`
- `status`
- `frame`
- `log`
- `ack`
- `error`
- `pong`

## Install Readiness

The board reports a coarse install-readiness state so the web UI can explain where the current setup is in the bring-up process:

- `bench-ready`
  board is alive and responding on the bench
- `installed-power-ready`
  board driver initialized and the wired install can move to vehicle power
- `runtime-ready`
  live CAN traffic is flowing

## Hardware Assumptions

The current defaults assume:

- Uno-compatible AVR board
- MCP2515 with `8 MHz` crystal
- optional HC-05 on `D4` and `D5`
- X179-powered install through the purchased converter

Important electrical constraints:

- do not drive HC-05 RX directly from the Uno without a divider
- do not feed X179 `12V` directly into the Uno `5V` pin
- disable MCP2515 termination on the live vehicle bus

## Tests

The native tests are host-side logic tests. They exercise command handling, board state, helper logic, and handler behavior without needing a real Uno connected.

The `uno` target is for the firmware build, not for those native tests.
