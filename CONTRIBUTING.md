# Contributing to TeslaCANModder

## Project Structure

```bash
TeslaCANModder/
├── firmware/          # PlatformIO firmware (C++, Arduino Uno + ESP32)
├── client/            # Unified Expo client app (browser, iOS, Android)
├── tools/             # Debug CLI (Node.js ESM)
├── packages/
│   └── protocol/      # Shared protocol types, commands, decoder (TypeScript)
├── docs/              # Documentation
└── legacy/            # Reference implementations from other projects
```

## Setup

### Prerequisites

- Node.js >= 18
- Python 3.11+ (for PlatformIO / firmware build server)
- PlatformIO CLI (`pip install platformio`)

### Install

```bash
npm install           # installs all workspaces
```

### Run Tests

```bash
npm run test:all      # all test suites
npm run test:firmware # PlatformIO native tests
npm run test:client   # client app tests
npm run test:protocol # shared protocol package tests
npm run test:tools    # CLI tools tests
```

### Development Servers

```bash
# Client app (Expo)
cd client && npm start

# Browser target
cd client && npm run web

# Firmware build server (Docker)
docker compose up firmware
```

## Coding Standards

### TypeScript / JavaScript

- Strict TypeScript where available (client, packages/protocol)
- ESM modules (`"type": "module"`)
- Functional React components with hooks
- No class components

### C++ (Firmware)

- Header-only library pattern in `firmware/lib/`
- 2-space indentation
- `#pragma once` header guards
- Build flags for feature gating (`BUS_*_ACTIVE`, `BOARD_ENABLE_*`)

## Checklist Expectations

Use the docs checklists as part of normal engineering review, not only at release time.

- `docs/checklists/can-review-checklist.md`: required when firmware changes can affect CAN frame mutation, routing, checksums, or transport-visible message shape.
- `docs/checklists/release-checklist.md`: required before tagged releases and artifact publishing.
- `docs/guides/quickstart-checklist.md`: review when changing setup, flashing, wiring, connection, or first-run flows.
- `docs/checklists/deprecation-checklist.md`: review when changing workspaces, CI, Docker, or docs-bundle generation so the repo stays consolidated around `client/`.
- `docs/checklists/testing-plan.md`: use its visual-regression section for UI-facing visual changes once the fixture + golden-image workflow is in place; until then, treat that section as the implementation plan for the workflow.

### Naming

- `camelCase` for JS/TS variables and functions
- `PascalCase` for React components and TS interfaces
- `UPPER_SNAKE_CASE` for C++ macros and constants

## Pull Request Process

1. Create a feature branch from `main`
2. Write tests for new functionality
3. Ensure all tests pass: `npm run test:all`
4. Ensure no lint errors: `npm run lint:all`
5. Keep commits focused — one logical change per commit
6. Open a PR with a clear description of changes

## Bus Architecture

The firmware supports 3 fixed CAN buses on the Tesla X179 connector:

| Bus | Index | X179 Pins | Build Flag | Default |
| --- | ----- | --------- | ---------- | ------- |
| Chassis | 0 | 13-14 | `BUS_CHASSIS_ACTIVE` | OFF when unset |
| Vehicle | 1 | 9-10 | `BUS_VEHICLE_ACTIVE` | OFF |
| Body | 2 | 2-3 | `BUS_BODY_ACTIVE` | OFF |

Bus activation is controlled by build flags. The build server or environment config injects these when compiling. `BUS_MAX` is always 3; `busActive(i)` checks if a bus is enabled. Shipping Uno / ESP32 environments enable the chassis bus explicitly in `firmware/platformio.ini`.

## Firmware Environments

| Environment | Board | Features |
| ----------- | ----- | -------- |
| `native` | Host | Tests only |
| `uno` | Arduino Uno | Serial |
| `uno_bt` | Arduino Uno | Serial + HC-05 Bluetooth |
| `esp32` | ESP32 DevKit | Serial |
| `esp32_wifi` | ESP32 DevKit | Serial + WiFi AP |
| `esp32_ble` | ESP32 DevKit | Serial + BLE |
| `esp32_wifi_ble` | ESP32 DevKit | Serial + WiFi + BLE |
