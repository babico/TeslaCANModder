# Contributing to TeslaCANModder

## Project Structure

```
TeslaCANModder/
├── hardware/          # PlatformIO firmware (C++, Arduino Uno + ESP32)
├── web/               # React web app (TypeScript, Vite)
├── mobile/            # React Native mobile app (TypeScript, Expo)
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
npm run test:firmware # PlatformIO native tests (9 suites)
npm run test:web      # web app tests
npm run test:mobile   # mobile app tests
npm run test:protocol # shared protocol package tests
npm run test:tools    # CLI tools tests
```

### Development Servers
```bash
# Web app (Vite dev server)
cd web && npm run dev

# Mobile app (Expo)
cd mobile && npm start

# Firmware build server (Docker)
docker compose up firmware
```

## Coding Standards

### TypeScript / JavaScript
- Strict TypeScript where available (web, mobile, packages/protocol)
- ESM modules (`"type": "module"`)
- Functional React components with hooks
- No class components

### C++ (Firmware)
- Header-only library pattern in `hardware/lib/`
- 2-space indentation
- `#pragma once` header guards
- Build flags for feature gating (`BUS_*_ACTIVE`, `BOARD_ENABLE_*`)

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
| --- | ----- | --------- | ---------- | -------| 
| FSD | 0 | 13-14 | `BUS_FSD_ACTIVE` | ON |
| Vehicle | 1 | 9-10 | `BUS_VEHICLE_ACTIVE` | OFF |
| Body | 2 | 2-3 | `BUS_BODY_ACTIVE` | OFF |

Bus activation is controlled by build flags. The build server injects these when compiling. `BUS_MAX` is always 3; `busActive(i)` checks if a bus is enabled.

## Firmware Environments

| Environment | Board | Features |
| ----------- | ----- | --------| 
| `native` | Host | Tests only |
| `uno` | Arduino Uno | Serial |
| `uno_bt` | Arduino Uno | Serial + HC-05 Bluetooth |
| `esp32` | ESP32 DevKit | Serial |
| `esp32_wifi` | ESP32 DevKit | Serial + WiFi AP |
| `esp32_ble` | ESP32 DevKit | Serial + BLE |
| `esp32_wifi_ble` | ESP32 DevKit | Serial + WiFi + BLE |
