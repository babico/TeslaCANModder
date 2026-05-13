---
name: testing-validation
description: Run tests and validation across all TeslaCANModder workspaces including firmware, protocol, client, and tools
license: GPL-3.0
compatibility: opencode
metadata:
    area: all
    stack: all
---

## What I do

Guide agents through running tests and validation in the TeslaCANModder monorepo. I cover:

- Test matrix for all four workspaces
- Linting, formatting, and type checking
- Adding tests for new behavior
- CI compliance checks
- Pre-completion validation workflow

## When to use me

Use this skill when:

- Running tests after any code change
- Setting up new test suites
- Validating code before claiming a task is done
- Debugging test failures
- Running CI checks locally

## Test matrix

| Layer    | Runner                    | Location             | Command                 |
| -------- | ------------------------- | -------------------- | ----------------------- |
| Firmware | PlatformIO Unity (native) | `firmware/test/`     | `npm run test:firmware` |
| Protocol | Jest (ESM)                | `packages/protocol/` | `npm run test:protocol` |
| Client   | Jest + Testing Library/RN | `client/tests/`      | `npm run test:client`   |
| Tools    | Jest (ESM)                | `tools/test/`        | `npm run test:tools`    |

## Running all tests

```bash
npm run test:all
```

This runs:

1. `npm run validate:serial-contract`
2. `npm run build:protocol`
3. `npm run test:protocol`
4. `npm run test:client`
5. `npm run test:tools`
6. `npm run test:firmware`

## Per-workspace tests

### Protocol package

```bash
npm run test:protocol
```

### Client app

```bash
npm run build:protocol && npm run test:client
```

### Tools CLI

```bash
npm run test:tools
```

### Firmware

```bash
npm run test:firmware
```

This runs `pio test -e native` inside `firmware/`. Native tests do not require hardware.

## Linting and formatting

```bash
# Lint + format check
npm run lint:all

# ESLint only
npm run lint

# Prettier check
npm run format:check

# Prettier write
npm run format
```

## Type checking

```bash
# Protocol
npm run typecheck:protocol

# Client
npm run typecheck:client
```

## Serial contract validation

```bash
npm run validate:serial-contract
```

## Firmware builds

Build firmware to verify compilation (run inside `firmware/`):

```powershell
cd firmware
.\pio.ps1 run -e esp32_chassis_8mhz
.\pio.ps1 run -e esp32_wifi_ble_chassis_vehicle_body_8mhz
```

## Adding tests

Rules for agents:

- Add tests with every behavior change.
- If a test framework is not wired up for the area you are adding (unlikely in this repo), set up the standard one for that stack before claiming the task is done.
- After any code change run the relevant per-workspace test command, and run `npm run lint:all` before finishing.

### Firmware tests

Tests are in `firmware/test/` using PlatformIO Unity.

Recommended suites:

- `test_native_helpers` - bit/frame helper assertions
- `test_native_hw3` - HW3 variant behavior
- `test_native_hw4` - HW4 variant behavior
- `test_native_legacy` - Legacy variant behavior
- `test_native_dispatch` - handler dispatch and routing
- `test_native_driver` - MCP2515 driver logic
- `test_native_serial` - serial command parsing
- `test_native_wifi` - WiFi REST API endpoints
- `test_native_persist` - EEPROM/NVS persistence

### Protocol tests

Tests are in `packages/protocol/test/` using Jest ESM.

Test:

- Command definitions
- Type constraints
- Decoder helpers
- Parsing edge cases

### Client tests

Tests are in `client/tests/` using Jest + React Native Testing Library.

Test:

- Component rendering and interactions
- State hook behavior
- Command dispatch
- Frame parsing

### Tools tests

Tests are in `tools/test/` using Jest ESM.

Test:

- CLI commands
- Utility functions
- Serial protocol parsing
- HTTP bridge behavior

## Before claiming a task is done

1. Build or compile the affected area (typecheck client/protocol, `pio run` for firmware).
2. Run the matching test command; add new tests for new behavior.
3. Run `npm run lint:all`.
4. Walk the relevant checklist in `docs/checklists/` if the change falls under its scope.
5. Clean up any temporary files or scratch scripts created during verification.

## CI pipeline

CI (GitHub Actions) runs on pushes to `main` and all PRs:

- Firmware native tests
- Protocol tests
- Client tests
- Tools tests
- Docker compose build

Keep changes green. Do not bypass husky + lint-staged hooks without explicit user approval.
