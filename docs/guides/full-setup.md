---
title: Full Setup Guide
title_tr: Birlesik Kurulum Rehberi
description: End-to-end setup flow for firmware flashing, first connection, and validation
category: guides
folder: guides
tags: [setup, unified, configuration]
order: 3
icon: 🧭
---

This is the canonical setup path for TeslaCANModder across firmware, transport, and client surfaces.

## Goal

- Flash the correct firmware image.
- Bring the board online over USB (and optionally WiFi/BLE).
- Verify board status and safety-critical baseline before vehicle installation.

## Prerequisites

- Node.js 18+
- npm workspaces installed from repository root (`npm install`)
- GitHub release access for prebuilt firmware assets, or PlatformIO available for local builds (`firmware/.pio.ps1`)
- Chrome or Edge for Web Serial flows
- Data-capable USB cable
- Correct MCP2515 crystal profile in hardware (8 MHz modules recommended)

## Step 1: Get Firmware Artifact

Preferred path: download the release binary built by GitHub Actions for your target connectivity and bus profile.

Common release asset names:

- `esp32.bin` — USB serial, chassis bus only
- `esp32_wifi.bin` — USB serial + WiFi, chassis bus only
- `esp32_ble_vehicle.bin` — USB serial + BLE, chassis + vehicle buses
- `esp32_wifi_ble_vehicle_body.bin` — USB serial + WiFi + BLE, chassis + vehicle + body buses

If you need a local custom build instead, choose your target environment and compile manually.

```powershell
cd firmware
.\.pio.ps1 run -e esp32_wifi_ble
```

Expected artifact example:

- ESP32: `firmware/.pio/build/esp32_wifi_ble/firmware.bin`

## Step 2: Flash Firmware (Unified Flasher Paths)

Use one of the following flasher paths. The browser flasher is best for first-time setup. CLI flasher is best for repeatable validation and scripted workflows.

### Path A: Browser Flasher (Client flow)

1. Start the client browser target:

```bash
cd client
npm install
npm run web
```

1. Open `http://localhost:5173` in Chrome/Edge.
1. Open Flasher tab.
1. Select connectivity and CAN bus profile.
1. Download the matching GitHub Release binary or run flash and wait for completion.
1. If ESP32 stalls, hold BOOT during upload start.

### Path B: CLI Flasher (tools command)

Use the debug CLI to flash and perform basic serial boot verification.

```bash
node tools/debug.js flash --port COM5 --hex firmware/.pio/build/esp32_wifi_ble/firmware.bin
```

Optional chip erase (factory reset style):

```bash
node tools/debug.js flash --port COM5 --hex firmware/.pio/build/esp32_wifi_ble/firmware.bin --erase
```

CLI flasher behavior:

- Resolves esptool from PlatformIO-managed paths first, then PATH.
- Flashes and attempts verification.
- Reopens serial and checks for boot/status output.

## Step 3: First Connection and Variant Set

After flashing:

1. Connect via USB in Dashboard or CLI.
2. Confirm boot/status output.
3. Set vehicle variant (`hw4`, `hw3`, `legacy`).

CLI example:

```bash
node tools/debug.js smoke --port COM5 --variant hw4
```

## Step 4: Baseline Validation Before Vehicle Install

Run these checks while bench-connected:

- `ping` returns `pong`.
- `status` returns expected variant and feature fields.
- Command ack path is healthy (`fsd:on`, `fsd:off`, `profile:1`).
- No repeated serial parse errors.

Suggested command:

```bash
node tools/debug.js test --port COM5 --variant hw4
```

## Step 5: Vehicle Install and Drive Evidence

Install on X179 harness, then run runtime validation capture for Drive in-review tasks:

```bash
node tools/debug.js drive-context --port COM5 --drive-duration 60000 --drive-min-samples 10 --drive-expect-full --drive-output artifacts/drive-context-report.json --drive-note-output artifacts/drive-context-note.txt
```

Expected closure target:

- `closureReadiness.d05 = true`
- `closureReadiness.d11 = true`
- `closureReadiness.d13 = true`
- `closureReadiness.allReady = true`

If not ready, use `closureChecklist.missingScenarioIds` to target missing ESP32-14 scenarios.

## Setup Decision Matrix

| Scenario                 | Recommended Path                        | Why                           |
| ------------------------ | --------------------------------------- | ----------------------------- |
| First board setup        | Browser flasher                         | Lowest friction, visual path  |
| Repeat bench flashing    | CLI Flasher                             | Fast, scriptable, easy to log |
| Release evidence run     | CLI Flasher + drive-context strict mode | Deterministic artifact output |
| Multi-board lab workflow | CLI Flasher                             | Repeatability and automation  |

## Common Failure Modes

| Symptom                    | Likely Cause                            | Fix                                            |
| -------------------------- | --------------------------------------- | ---------------------------------------------- |
| Board not detected         | Charge-only USB cable or driver missing | Use data cable, install CH340/CP210x driver    |
| Flash fails immediately    | Wrong port or esptool not available     | Verify COM port and PlatformIO toolchain       |
| ESP32 upload hangs         | BOOT timing issue                       | Hold BOOT during upload start                  |
| No status after flash      | Baud mismatch or board reset loop       | Verify 115200 and power integrity              |
| Strict drive-context fails | Missing run scenarios                   | Re-run targeted ESP32-14 scenarios from report |

## Related Docs

- [Getting Started](getting-started.md)
- [Quickstart Checklist](quickstart-checklist.md)
- [Unified Client Guide](../architecture/unified-client-guide.md)
- [E2E Testing Plan](../checklists/testing-plan.md)
- [Release Checklist](../checklists/release-checklist.md)
