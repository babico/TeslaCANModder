---
name: firmware-development
description: PlatformIO ESP32 firmware development for TeslaCANModder including build environments, MCP2515 CAN bus configuration, feature handlers, and C++ coding standards
license: GPL-3.0
compatibility: opencode
metadata:
    area: firmware
    stack: cpp
---

## What I do

Guide agents through the TeslaCANModder PlatformIO ESP32 firmware codebase in `firmware/`. I cover:

- Building and flashing firmware with `pio.ps1`
- Understanding the three MCP2515 CAN buses (Chassis, Vehicle, Body)
- Adding feature handlers and frame mutation logic
- Writing C++ code that follows `.clang-format` and `.editorconfig`
- Running native host tests with PlatformIO Unity

## When to use me

Use this skill when:

- Modifying or adding C++ code in `firmware/`
- Adding new CAN features or handlers
- Changing MCP2515 driver or bus configuration
- Working with ESP32 hardware abstraction
- Adding or modifying firmware tests

## Project structure

```
firmware/
├── lib/core/           shared types, driver, persistence, log, CAN plumbing
│   ├── core/can/       MCP2515 bus, health, filters, recorder, ring buffer
│   ├── core/config/    runtime config (esp32/)
│   ├── core/driver/    platform drivers (esp32/)
│   ├── core/log/       log ring
│   └── core/util/      parse helpers
├── lib/io/             transports: serial (USB+BT), WiFi, BLE
├── lib/client/         on-device surfaces: REST API, dashboard, command dispatch, gamepad
│   ├── client/api/     auth, routes, init
│   ├── client/command/ dispatch, features, messages
│   └── client/dashboard/ HTML dashboard served over WiFi
├── lib/vehicle/can/    Tesla CAN logic
│   ├── vehicle/can/feature/  feature command handlers and frame mutation
│   └── vehicle/can/handler/  variant-specific frame handlers and dispatch
├── lib/vehicle/ble/    Tesla BLE protocol
│   ├── vehicle/ble/feature/  BLE feature handlers
│   └── vehicle/ble/handler/  BLE dispatch
├── src/esp32/          ESP32 firmware entry point
├── test/               native PlatformIO regression suites
├── platformio.ini      environment + build flag configuration
└── pio.ps1             local wrapper around PlatformIO CLI (Windows)
```

## Coding standards

- `.clang-format` is the source of truth: LLVM base, tabs (width 4), Allman braces, right-aligned pointers/references, `SortIncludes: false`, column limit 120.
- `.editorconfig` confirms tab indent for `*.cpp,h,c,ino`.
- Header-only library pattern in `firmware/lib/`.
- `#pragma once` header guards.
- Feature gating via build flags (`BUS_*_ACTIVE`, `BOARD_ENABLE_*`).
- `UPPER_SNAKE_CASE` for C++ macros and constants.

## Environment naming convention

```
esp32[_wifi][_ble][_chassis][_vehicle][_body][_8mhz|_16mhz]
```

Segments (all optional, order fixed):

- `_wifi` BOARD_ENABLE_WIFI=1 REST API + dashboard
- `_ble` BOARD_ENABLE_BLE=1 NimBLE UART NUS + gamepad central
- `_chassis` BUS_CHASSIS_ACTIVE=1 Chassis CAN (X179 pins 13-14)
- `_vehicle` BUS_VEHICLE_ACTIVE=1 Vehicle Ctrl CAN (X179 pins 9-10)
- `_body` BUS_BODY_ACTIVE=1 Body Ctrl CAN (X179 pins 2-3)
- `_8mhz` BOARD_CAN_CLOCK_MHZ=8 8 MHz MCP2515 crystal (most modules)
- `_16mhz` BOARD_CAN_CLOCK_MHZ=16 16 MHz MCP2515 crystal (some modules)

## Bus wiring reference

| Bus     | MCP2515 | CS  | INT | X179 Pins | Function                |
| ------- | ------- | --- | --- | --------- | ----------------------- |
| Chassis | #1      | 15  | 34  | 13-14     | Chassis / Autopilot CAN |
| Vehicle | #2      | 27  | 35  | 9-10      | Vehicle Control CAN     |
| Body    | #3      | 26  | 33  | 2-3       | Body Control CAN        |

All buses run at 500 kbps. `BUS_MAX` is always 3; use `busActive(i)` to check if a bus is enabled.

## Common commands

Build (run inside `firmware/`):

```powershell
cd firmware
.\pio.ps1 run -e esp32_chassis_8mhz
.\pio.ps1 run -e esp32_wifi_ble_chassis_vehicle_body_8mhz
```

Test (native host tests, no hardware needed):

```powershell
cd firmware
.\pio.ps1 test -e native
```

Enable all buses for local build:

```powershell
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_CHASSIS_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\pio.ps1 run -e esp32_wifi_ble_chassis_vehicle_body_8mhz
```

Monitor serial output:

```powershell
cd firmware
.\pio.ps1 device monitor
```

## Feature patterns

Feature handlers live in `firmware/lib/vehicle/can/feature/`.

Common patterns:

- **Toggle-Inject**: ON -> add filter + intercept + modify + send, OFF -> remove filter (Bus 0 / FSD)
- **Echo-Inject**: Read frame -> clone -> modify -> send back on same bus (Bus 1 / Vehicle)
- **Burst-Inject**: Build frame -> `startBurst(count, delayMs)` non-blocking (Bus 1 or 2)
- **Tick-Inject**: Dedicated timer loop sends frames indefinitely or with countdown (Bus 1 / Vehicle)
- **Read-Only**: Decode frame -> update state (no send) (Bus 1 / Vehicle)
- **Config-Only**: Update state + persist (no CAN interaction)

## Safety rules

- `txPaused` gates ALL transmission paths when OTA update is detected.
- Every direct `frame.data[...]` read has a matching DLC guard before access.
- Every direct `frame.data[...]` write has a matching DLC guard before mutation.
- Existing checksum logic must be recalculated when a changed byte requires it.
- Bit writes only touch the intended field and do not leak into adjacent bits.

## Testing

Tests are in `firmware/test/` using PlatformIO Unity.

Recommended test suites:

- `test_native_helpers` - bit/frame helper assertions
- `test_native_hw3` - HW3 variant behavior
- `test_native_hw4` - HW4 variant behavior
- `test_native_legacy` - Legacy variant behavior
- `test_native_dispatch` - handler dispatch and routing
- `test_native_driver` - MCP2515 driver logic
- `test_native_serial` - serial command parsing
- `test_native_wifi` - WiFi REST API endpoints
- `test_native_persist` - EEPROM/NVS persistence

Always add tests for:

- exact bit-set / bit-clear assertion for changed fields
- short-frame regression test proving no send happens when DLC is too small
- variant-specific regression test proving unsupported variants do not drift
- stream/message-shape assertion when board output changes

## Before finishing

1. Build the affected environment(s) with `pio run`.
2. Run `pio test -e native` and ensure all tests pass.
3. Run `npm run lint:all` from repo root.
4. Walk `docs/checklists/can-review-checklist.md` if the change affects CAN frame mutation, routing, checksums, or transport-visible message shape.
5. Clean up any temporary files.

## Key files to reference

- `firmware/platformio.ini` - environments and build flags
- `firmware/lib/core/can/` - MCP2515 bus plumbing
- `firmware/lib/vehicle/can/feature/` - feature handlers
- `firmware/lib/vehicle/can/handler/` - variant dispatch
- `docs/reference/can-protocol.md` - CAN IDs and frame structures
- `docs/reference/commands.md` - command reference
- `docs/architecture/feature-workflows.md` - feature patterns
