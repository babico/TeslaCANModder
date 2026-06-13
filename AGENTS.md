# AGENTS.md

Instructions for AI coding agents working in the TeslaCANModder repo.

This file is the single entry point for agents. It complements `README.md` and `CONTRIBUTING.md` and defers to `docs/` for deep reference material. Read this file first, then follow the links.

## What this repo is

Tesla CAN firmware, client, protocol, and diagnostics tooling centered on ESP32-S DevKit hardware and the Tesla X179 connector. License: GPL-3.0.

Four maintained areas, all in one npm workspace monorepo plus a PlatformIO firmware tree:

| Area                 | Purpose                                                                              | Tooling                  |
| -------------------- | ------------------------------------------------------------------------------------ | ------------------------ |
| `firmware/`          | PlatformIO ESP32 firmware with 1-3 MCP2515 buses, WiFi REST API, and BLE             | PlatformIO, C++, Unity   |
| `client/`            | Expo client for web, iOS, and Android, including the browser flasher and in-app docs | React Native, Expo, Jest |
| `packages/protocol/` | Shared protocol types, commands, decoder data, parsing helpers                       | TypeScript, Jest (ESM)   |
| `tools/`             | Debug CLI and serial-to-HTTP bridge for bench work and validation                    | Node.js ESM, Jest        |

The documentation screen in the client renders the raw markdown from `docs/` directly. There is no separate generated docs bundle.

`legacy/` holds read-only git submodules of 80+ external community repos for research. Do not copy legacy code into shipping code. Per-repo analyses live in `docs/legacy/`.

## Repository layout

```text
TeslaCANModder/
├── firmware/                      PlatformIO ESP32 firmware (C++)
│   ├── lib/core/                  shared types, driver, persistence, log, CAN plumbing
│   │   ├── core/can/              MCP2515 bus, health, filters, recorder, ring buffer
│   │   ├── core/config/           runtime config (esp32/)
│   │   ├── core/driver/           platform drivers (esp32/)
│   │   ├── core/log/              log ring
│   │   └── core/util/             parse helpers
│   ├── lib/io/                    transports: serial (USB+BT), WiFi, BLE
│   ├── lib/client/                on-device surfaces: REST API, dashboard, command dispatch, gamepad
│   │   ├── client/api/            auth, routes, init
│   │   ├── client/command/        dispatch, features, messages
│   │   └── client/dashboard/      HTML dashboard served over WiFi
│   ├── lib/vehicle/can/           Tesla CAN logic
│   │   ├── vehicle/can/feature/   feature command handlers and frame mutation
│   │   └── vehicle/can/handler/   variant-specific frame handlers and dispatch
│   ├── lib/vehicle/ble/           Tesla BLE protocol
│   │   ├── vehicle/ble/feature/   BLE feature handlers (session, key, …)
│   │   └── vehicle/ble/handler/   BLE dispatch
│   ├── src/esp32/                 ESP32 firmware entry point
│   ├── test/                      native PlatformIO regression suites
│   ├── platformio.ini             environment + build flag configuration
│   └── pio.ps1                    local wrapper around PlatformIO CLI (Windows)
├── client/                        Expo app (browser, iOS, Android)
│   ├── src/                       app source (components, screens, state, hardware, ui, …)
│   └── tests/                     Jest + Testing Library tests
├── packages/protocol/             @teslacanmodder/protocol (shared TS package)
├── tools/                         debug CLI (tcm-debug), serial-to-HTTP bridge
├── docs/                          canonical markdown docs (also rendered in the client)
├── legacy/                        external reference submodules (read only)
└── scripts/                       workspace-level validation and smoke scripts
```

The firmware tree was reorganized recently; older docs that reference `lib/infra/`, `lib/feature/`, or `lib/handler/` at the top level of `firmware/lib` are stale. The shipping equivalents now live under `lib/core/can/`, `lib/vehicle/can/feature/`, and `lib/vehicle/can/handler/` respectively.

## Start-here docs

When a task touches an area, read the matching doc first:

- First install and flashing: `docs/guides/getting-started.md`, `docs/guides/full-setup.md`, `docs/guides/flasher-quickstart.md`, `docs/guides/hardware-setup.md`
- Commands, CAN IDs, protocol: `docs/reference/commands.md`, `docs/reference/can-ids.md`, `docs/reference/can-protocol.md`, `docs/reference/signal-matrix.md`, `docs/reference/state-fields.md`
- Client architecture and UX: `docs/architecture/unified-client-guide.md`, `docs/architecture/monitor-architecture.md`, `docs/architecture/feature-workflows.md`, `docs/architecture/layout-system.md`
- Release and QA: `docs/checklists/release-checklist.md`, `docs/checklists/testing-plan.md`, `docs/checklists/can-review-checklist.md`, `docs/checklists/deprecation-checklist.md`
- Troubleshooting: `docs/troubleshooting/debug-guide.md`
- Research and comparisons: `docs/legacy/README.md`, `docs/legacy/COMPARISON.md`

Agent rule: if you make a change that a checklist covers, follow that checklist before declaring the task done. `can-review-checklist.md` is required for any firmware change that can affect CAN frame mutation, routing, checksums, or transport-visible message shape.

## Setup

Prerequisites:

- Node.js >= 18
- Python 3.11+ (for PlatformIO)
- PlatformIO CLI (`pip install platformio`) when touching firmware

Install all workspace deps from the repo root:

```bash
npm install
```

## Commands an agent should know

All scripts run from the repo root unless noted. The shell on this workspace is `bash` on Windows; PlatformIO uses the `pio.ps1` wrapper in `firmware/`.

Validation:

| Task                     | Command                            |
| ------------------------ | ---------------------------------- |
| Run everything           | `npm run test:all`                 |
| Lint + format check      | `npm run lint:all`                 |
| ESLint only              | `npm run lint`                     |
| Prettier check           | `npm run format:check`             |
| Prettier write           | `npm run format`                   |
| Typecheck protocol       | `npm run typecheck:protocol`       |
| Typecheck client         | `npm run typecheck:client`         |
| Validate serial contract | `npm run validate:serial-contract` |

Per-workspace tests:

| Target                             | Command                 |
| ---------------------------------- | ----------------------- |
| Protocol package (Jest ESM)        | `npm run test:protocol` |
| Client app (Jest + RN Testing Lib) | `npm run test:client`   |
| Tools CLI (Jest ESM)               | `npm run test:tools`    |
| Firmware (PlatformIO native)       | `npm run test:firmware` |

Firmware builds (run inside `firmware/` using the `pio.ps1` wrapper):

```powershell
cd firmware
.\pio.ps1 run -e esp32_chassis_8mhz
.\pio.ps1 run -e esp32_wifi_chassis_8mhz
.\pio.ps1 run -e esp32_ble_chassis_8mhz
.\pio.ps1 run -e esp32_wifi_ble_chassis_8mhz
.\pio.ps1 test -e native
```

Client development (run manually, do not start as a background task from an agent action):

```bash
npm run web -w @teslacanmodder/client
# or
cd client && npm start
```

Never start `expo start`, `npm run web`, `pio monitor`, or other long-running watchers from an automated tool run. Suggest them as manual commands for the user.

## Firmware targets and bus flags

Environments in `firmware/platformio.ini`:

| Environment                   | Board        | Features                      |
| ----------------------------- | ------------ | ----------------------------- |
| `native`                      | Host         | Tests only                    |
| `esp32_chassis_8mhz`          | ESP32 DevKit | Serial + Chassis CAN          |
| `esp32_wifi_chassis_8mhz`     | ESP32 DevKit | Serial + WiFi + Chassis       |
| `esp32_ble_chassis_8mhz`      | ESP32 DevKit | Serial + BLE + Chassis        |
| `esp32_wifi_ble_chassis_8mhz` | ESP32 DevKit | Serial + WiFi + BLE + Chassis |

Bus lanes are controlled with build flags on the Tesla X179 connector:

| Bus     | Index | X179 Pins | Build Flag           | Default                    |
| ------- | ----- | --------- | -------------------- | -------------------------- |
| Chassis | 0     | 13-14     | `BUS_CHASSIS_ACTIVE` | On for shipping ESP32 envs |
| Vehicle | 1     | 9-10      | `BUS_VEHICLE_ACTIVE` | Off                        |
| Body    | 2     | 2-3       | `BUS_BODY_ACTIVE`    | Off                        |

`BUS_MAX` is always 3; use `busActive(i)` to check if a bus is enabled. Enable extra lanes for a local build with:

```powershell
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_CHASSIS_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\pio.ps1 run -e esp32_wifi_ble_chassis_8mhz
```

Tagged releases publish merged flash-ready images through GitHub Actions; the client flasher consumes those assets directly.

## Coding standards

### TypeScript / JavaScript (client, packages/protocol, tools)

- Strict TypeScript where available.
- ESM modules; `packages/protocol` and `tools` are `"type": "module"`.
- Functional React components with hooks. No class components.
- ESLint rules enforced (`eslint.config.mjs`):
    - `eqeqeq` strict — always `===` / `!==`.
    - `no-console` — only `console.warn` and `console.error` are allowed; CLI code under `tools/**/*.{js,mjs}` is exempt.
    - `@typescript-eslint/no-explicit-any` warns; tests are exempt.
    - Unused vars: prefix with `_` to silence.
    - `no-eval` and `no-implied-eval` are errors.
- Prettier settings (`.prettierrc`): tabs, tab width 4, print width 100, semicolons, double quotes, trailing commas everywhere, LF line endings, always-parenthesized arrow params. YAML uses 4-space indent.

### C++ (firmware)

- `.clang-format` is the source of truth: LLVM base, tabs (width 4), Allman braces, right-aligned pointers/references, `SortIncludes: false`, column limit 120.
- `.editorconfig` confirms tab indent for `*.cpp,h,c,ino`. If `CONTRIBUTING.md` mentions 2-space indent for C++, follow `.clang-format` and `.editorconfig` instead — tabs width 4 is the authoritative style.
- Header-only library pattern in `firmware/lib/`.
- `#pragma once` header guards.
- Feature gating via build flags (`BUS_*_ACTIVE`, `BOARD_ENABLE_*`).

### Naming

- `camelCase` for JS/TS variables and functions.
- `PascalCase` for React components and TS interfaces.
- `UPPER_SNAKE_CASE` for C++ macros and constants.

## Testing expectations

| Layer    | Runner                    | Location             |
| -------- | ------------------------- | -------------------- |
| Firmware | PlatformIO Unity (native) | `firmware/test/`     |
| Protocol | Jest (ESM)                | `packages/protocol/` |
| Client   | Jest + Testing Library/RN | `client/tests/`      |
| Tools    | Jest (ESM)                | `tools/test/`        |

Rules for agents:

- Add tests with every behavior change. If a test framework is not wired up for the area you are adding (unlikely in this repo), set up the standard one for that stack before claiming the task is done.
- After any code change run the relevant per-workspace test command, and run `npm run lint:all` before finishing.
- Before a firmware release or any change touching transport-visible behavior, run `npm run test:all` and walk `docs/checklists/release-checklist.md`.

## Safety, git, and CI

- CI (GitHub Actions) runs on pushes to `main` and all PRs: firmware native tests, protocol tests, client tests, tools tests, and the docker compose build. Keep changes green.
- Husky + lint-staged run Prettier and ESLint `--fix` on staged JS/TS and Prettier on JSON/MD/YAML. Do not bypass hooks without explicit user approval; do not pass `--no-verify`.
- Do not commit or push unless the user explicitly asks. Never push to `main` directly; always use a feature branch with `-u` tracking.
- Flag any file that looks like it contains secrets (`.env`, credential files) before committing.
- The `legacy/` tree is read-only reference. Never modify files under `legacy/`. Comparative notes live in `docs/legacy/`.

## MCP Servers

Model Context Protocol (MCP) servers extend OpenCode with additional tools for this project.

| Server                 | Package                                           | Purpose                                     | Status              |
| ---------------------- | ------------------------------------------------- | ------------------------------------------- | ------------------- |
| **github**             | `@github/github-mcp-server`                       | GitHub operations (issues, PRs, repos)      | Disabled by default |
| **playwright**         | `@playwright/mcp`                                 | Browser automation for testing web client   | Disabled by default |
| **memory**             | `@modelcontextprotocol/server-memory`             | Persistent memory across sessions           | Disabled by default |
| **sequentialthinking** | `@modelcontextprotocol/server-sequentialthinking` | Step-by-step reasoning for complex analysis | Disabled by default |
| **time**               | `@modelcontextprotocol/server-time`               | Time utilities and scheduling               | Disabled by default |
| **superpowers**        | `superpowers-mcp`                                 | General utility enhancements                | Disabled by default |

All MCP servers are **disabled by default** to save context. Enable in `opencode.json`:

```json
{
    "mcp": {
        "github": { "enabled": true }
    }
}
```

Environment variables: `GITHUB_TOKEN` required for GitHub MCP.

## What to do before claiming a task is done

1. Build or compile the affected area (typecheck client/protocol, `pio run` for firmware).
2. Run the matching test command; add new tests for new behavior.
3. Run `npm run lint:all`.
4. Walk the relevant checklist in `docs/checklists/` if the change falls under its scope.
5. Clean up any temporary files or scratch scripts created during verification.
