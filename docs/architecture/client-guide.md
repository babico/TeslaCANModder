---
title: Client Guide
description: Canonical client architecture and legacy surface migration reference
category: architecture
folder: architecture
tags: [client, architecture, migration]
order: 8
---

# Unified Client Guide

This guide documents the canonical client experience in `client/`.

## Scope

- Canonical app surface: `client/` (React Native + Expo).
- Browser, iOS, and Android targets all ship from the same workspace.
- Shared protocol/state primitives come from `packages/protocol`.

## Architecture

### Runtime layers

1. Transport + command execution:
    - Implemented in `client/src/hardware/controller.ts`.
    - Uses protocol commands from `@teslacanmodder/protocol`.

1. Shared board state domain:
    - Uses `initialBoardState` and reducer-compatible payload ingestion.
    - App-side ingestion in `client/src/state/board.ts`.

1. Unified shell and tabs:
    - `Drive`, `Controls`, `Monitor` are orchestrated in `client/src/AppExperience.tsx`.

1. Feature modules:
    - Drive cluster: `client/src/components/DriveScreen.tsx`.
    - Controls surface: `client/src/components/ControlsScreen.tsx`.
    - Monitor diagnostics panels: telemetry/integration/utility panels under `client/src/components/`.

## Setup

### Prerequisites

- Node.js 18+
- npm workspaces enabled (`npm install` at repo root)

### Run unified client

```bash
cd client
npm install
npm run start
```

Optional targets:

```bash
npm run web
npm run android
npm run ios
```

### Validation

```bash
cd client
npx tsc --noEmit
```

### Firmware and Flasher Setup

Client workflows assume firmware is already built and flashed.

- For the canonical end-to-end setup path (build, flash, first connect, strict validation), use [Unified Setup Guide](unified-setup-guide.md).
- For repeatable CLI flashing and board verification, use `node tools/debug.js flash --port COMX --hex <path-to-hex>` from repository root.

### Runtime Connection Transports

The client controller now supports explicit connection paths for:

- REST API (WiFi HTTP bridge)
- BLE
- Socket bridge (TCP/WebSocket style request-response)
- COM serial port
- Bluetooth COM serial port (SPP/RFCOMM)

Controller methods:

- `connectViaRestApi(...)`
- `connectViaBle(...)`
- `connectViaSocket(...)`
- `connectViaComPort(...)`
- `connectViaBluetoothComPort(...)`

## Drive Interactions

Drive surface behavior is centered on live board-state rendering:

- Speed/gear/power/charge telemetry updates from shared state.
- Day/night and gauge mode controls are available from the drive UI.
- Side gauges and AP/indicator visuals are data-driven and feature-aware.

## Monitor Workflows

Monitor now includes converged workflows:

- Frame feed controls: pause/resume, bus/text filtering.
- Performance controls: windowing, sampling, and in-app stress validation.
- Decoder workflow: dataset selection, frame selection, decoded details.
- Export workflow:
- raw session: JSON / JSONL
- decoded session: JSON / CSV
- dataset-derived DBC preview
- provenance/session packaging metadata embedded in exports.

## Maintenance Notes

- New features should target `client/`.
- Avoid reintroducing separate app surfaces for browser-specific or native-specific behavior; keep that logic inside the shared client workspace.
- When adding monitor/controls/drive behavior, include compile evidence (`npx tsc --noEmit`).

## Related Docs

- `README.md`
- `docs/unified-setup-guide.md`
- `docs/getting-started.md`
- `docs/feature-workflows.md`
- `docs/legacy-summary.md`
